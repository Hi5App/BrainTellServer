package utils

import (
	"errors"
	_ "github.com/go-sql-driver/mysql"
	"github.com/gomodule/redigo/redis"
	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
	log "github.com/sirupsen/logrus"
	"github.com/spf13/viper"
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
var Tmpdir string
var ImageDir string
var AesKey string
var CropProcess int64
var Emails []string

func LoadConfig() error {
	//配置系统日志
	path := "systemlog"
	writer, _ := rotatelogs.New(
		path+".%Y%m%d%H%M",
		rotatelogs.WithLinkName(path),
		rotatelogs.WithMaxAge(7*24*time.Hour),
		rotatelogs.WithRotationTime(24*time.Hour),
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
	DataPath = MainPath + "/data"
	Tmpdir = MainPath + "/tmp"
	ImageDir = MainPath + "/image"

	CropProcess = config.GetInt64("cropprocess")
	Emails = config.GetStringSlice("emails")
	return nil
}

func NewDb(user, passwd, ip, port, db string) error {
	if len(user) == 0 || len(ip) == 0 || len(passwd) == 0 || len(db) == 0 || len(port) == 0 {
		log.WithFields(log.Fields{
			"event": "Allocate DB",
		}).Infoln("Config is Error")
		return errors.New("config is Error")
	}
	DB, err := xorm.NewEngine("mysql", user+":"+passwd+"@tcp("+ip+":"+port+")/"+db)
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
