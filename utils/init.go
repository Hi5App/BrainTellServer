package utils

import (
	"context"
	"errors"
	"strconv"
	"time"

	_ "github.com/go-sql-driver/mysql"
	"github.com/gomodule/redigo/redis"
	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
	log "github.com/sirupsen/logrus"
	"github.com/spf13/viper"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"golang.org/x/sync/semaphore"
	"xorm.io/xorm"
)

type MySQLConfig struct {
	User   string
	Passwd string
	IP     string
	Port   string
	DB     string
}

type RedisConfig struct {
	IP   string
	Port string
}

type Config struct {
	MySQL       MySQLConfig
	Redis       RedisConfig
	AesKey      string
	MainPath    string
	CropProcess int64
	Emails      []string
}

type MongoDbConnectionCreateInfo struct {
	Host     string
	Port     int32
	User     string
	Password string
}

type MongoDbConnectionInfo struct {
	Client *mongo.Client
	Err    error
}

var DB *xorm.Engine
var Pool *redis.Pool
var MainPath string
var Vaa3dBin string
var DataPath string
var CollaborateBinPath string
var Tmpdir string
var ImageDir string
var AesKey string
var availableCropProcess *semaphore.Weighted
var Emails []string
var QueueSize int

var UserDB *mongo.Collection

// coll configs
var MaxRoomConnections int
var MaxJobs int
var AIInterval int

// rating service
var RatingImagePath string

func LoadConfig(configName string) error {
	//读取系统配置文件
	config := viper.New()
	config.AddConfigPath("./")
	config.SetConfigName(configName)
	config.SetConfigType("json")

	if err := config.ReadInConfig(); err != nil {
		log.WithFields(log.Fields{
			"event": "Load config",
		}).Fatalln("Failed to send event")
	}

	logpath := config.GetString("logpath")
	if logpath == "" {
		logpath = "./logs/systemlog"
	}
	log.Printf("logpath:%s", logpath)

	//配置系统日志
	path := logpath
	writer, _ := rotatelogs.New(
		path+".%Y%m%d%H%M",
		rotatelogs.WithLinkName(path),
		rotatelogs.WithMaxAge(30*24*time.Hour),
		rotatelogs.WithRotationTime(1*time.Hour),
	)
	log.SetOutput(writer)
	customFormatter := new(log.TextFormatter)
	customFormatter.TimestampFormat = "2006-01-02 15:04:05"
	customFormatter.FullTimestamp = true
	log.SetFormatter(customFormatter)
	log.SetLevel(log.TraceLevel)
	log.Infoln("Log Set Up")

	//建立MySQL连接池
	if err := NewDb(config.GetString("mysql.user"),
		config.GetString("mysql.passwd"),
		config.GetString("mysql.ip"),
		config.GetString("mysql.port"),
		config.GetString("mysql.db"),
	); err != nil {
		return err
	}
	//初始化Redis
	if err := NewRedisPool(config.GetString("redis.ip"),
		config.GetString("redis.port")); err != nil {
		return err
	}

	AesKey = config.GetString("aeskey")
	MainPath = config.GetString("mainpath")
	Vaa3dBin = MainPath + "/vaa3d/cropimage"
	DataPath = MainPath + "/testdata"
	Tmpdir = MainPath + "/tmp"
	ImageDir = MainPath + "/image"
	CollaborateBinPath = config.GetString("collserverpath")
	availableCropProcess = semaphore.NewWeighted(config.GetInt64("cropprocess"))
	//没有这个key时，QueueSize=0
	QueueSize = config.GetInt("queuesize")
	Emails = config.GetStringSlice("emails")

	// coll configs
	MaxJobs = config.GetInt("collaboration.max_jobs")
	MaxRoomConnections = config.GetInt("collaboration.max_connections") / MaxJobs
	AIInterval = config.GetInt("collaboration.ai_interval")

	// mongodb connection
	// connect to mongodb for user data management
	createInfo := MongoDbConnectionCreateInfo{
		Host:     config.GetString("mongodb.ip"),
		Port:     int32(config.GetInt("mongodb.port")),
		User:     config.GetString("mongodb.user"),
		Password: config.GetString("mongodb.password"),
	}
	InitializeMongodbConnection(createInfo)

	// rating service config
	RatingImagePath = config.GetString("ratingservice.ratingimagepath")

	log.WithFields(log.Fields{
		"event": "Init coll config",
	}).Infof("max jobs: %v, max room_con: %v, ai interval: %v\n", MaxJobs, MaxRoomConnections, AIInterval)

	// 初始化redis中的数据
	if configName == "config_dynamic" {
		if err := InitRedisData(); err != nil {
			log.WithFields(log.Fields{
				"event": "Redis",
				"desc":  "Init Redis data failed",
			}).Warnf("%v\n", err)
		}
	}

	return nil
}

func NewDb(user, passwd, ip, port, db string) error {
	if len(user) == 0 || len(ip) == 0 || len(passwd) == 0 || len(db) == 0 || len(port) == 0 {
		log.WithFields(log.Fields{
			"event": "Allocate DB",
		}).Infoln("Config is Error")
		return errors.New("config is Error")
	}
	var err error
	DB, err = xorm.NewEngine("mysql", user+":"+passwd+"@tcp("+ip+":"+port+")/"+db)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Allocate DB",
		}).Infof("%v\n", err)
		return err
	}

	DB.SetMaxOpenConns(1000)
	DB.SetMaxIdleConns(200)
	DB.ShowSQL(true)
	return nil
}

func NewRedisPool(ip, port string) error {
	if len(ip) == 0 || len(port) == 0 {
		log.WithFields(log.Fields{
			"event": "Allocate Redis Pool",
		}).Infoln("Config is Error")
		return errors.New("config is Error")
	}

	Pool = &redis.Pool{
		MaxIdle:     5,
		IdleTimeout: 240 * time.Second,
		Dial:        func() (redis.Conn, error) { return redis.Dial("tcp", ip+":"+port) },
	}
	return nil
}

func InitRedisData() error {
	// 初始化redis数据
	// todo 其他redis数据的初始化
	conn := Pool.Get()
	defer conn.Close()

	// 初始化端口数据
	_, err := conn.Do("DEL", "PORTQUEUE")
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "DELETE old PORT QUEUE failed",
		}).Warnf("%v\n", err)
	}

	// 删除旧链接

	for i := 0; i < MaxJobs*MaxRoomConnections; i++ {
		_, err := conn.Do("RPUSH", "PORTQUEUE", strconv.Itoa(4000+i))
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Redis",
				"desc":  "Init PORT QUEUE failed",
			}).Warnf("%v\n", err)
		}
	}

	return nil
}

func InitializeMongodbConnection(createInfo MongoDbConnectionCreateInfo) {
	url := "mongodb://" + createInfo.Host + ":" + strconv.Itoa(int(createInfo.Port))
	var connectionInfo MongoDbConnectionInfo

	credential := options.Credential{
		Username: createInfo.User,
		Password: createInfo.Password,
	}
	clientOpts := options.Client().ApplyURI(url).
		SetAuth(credential).SetConnectTimeout(10 * time.Second)

	connectionInfo.Client, connectionInfo.Err = mongo.Connect(context.TODO(), clientOpts)
	if connectionInfo.Err != nil {
		log.Fatal(connectionInfo.Err)
	}

	var err = connectionInfo.Client.Ping(context.TODO(), nil)

	if err != nil {
		log.Fatal(err)
		panic("Connect to mongodb failed!")
	}

	metaInfoDb := connectionInfo.Client.Database("MetaInfoDataBase")
	UserDB = metaInfoDb.Collection("UserMetaInfoCollection")
	log.Printf("Connect to mongodb successfully!")
}
