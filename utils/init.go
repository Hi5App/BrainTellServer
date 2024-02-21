package utils

import (
	"errors"
	_ "github.com/go-sql-driver/mysql"
	"github.com/gomodule/redigo/redis"
	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
	log "github.com/sirupsen/logrus"
	"github.com/spf13/viper"
	"golang.org/x/sync/semaphore"
	"strconv"
	"time"
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

// coll configs
var MaxRoomConnections int
var MaxJobs int
var AIInterval int

func LoadConfig() error {

	//配置系统日志
	path := "./testlogs/systemlog"
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

	//读取系统配置文件
	config := viper.New()
	config.AddConfigPath("./")
	config.SetConfigName("config")
	config.SetConfigType("json")

	if err := config.ReadInConfig(); err != nil {
		log.WithFields(log.Fields{
			"event": "Load config",
		}).Fatalln("Failed to send event")
	}

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
	CollaborateBinPath = MainPath + "/collaborate/CollServerTest"
	availableCropProcess = semaphore.NewWeighted(config.GetInt64("cropprocess"))
	//没有这个key时，QueueSize=0
	QueueSize = config.GetInt("queuesize")
	Emails = config.GetStringSlice("emails")

	// coll configs
	MaxJobs = config.GetInt("collaboration.max_jobs")
	MaxRoomConnections = config.GetInt("collaboration.max_connections") / MaxJobs
	AIInterval = config.GetInt("collaboration.ai_interval")

	log.WithFields(log.Fields{
		"event": "Init coll config",
	}).Infof("max jobs: %v, max room_con: %v, ai interval: %v\n", MaxJobs, MaxRoomConnections, AIInterval)

	// 初始化redis中的数据
	if err := InitRedisData(); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Init Redis data failed",
		}).Warnf("%v\n", err)
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
