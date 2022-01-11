package utils

import (
	"errors"
	_ "github.com/go-sql-driver/mysql"
	"github.com/gomodule/redigo/redis"
	log "github.com/sirupsen/logrus"
	"github.com/spf13/viper"
	"os"
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
	MySQL    MySQLConfig
	Redis    RedisConfig
	AesKey   string
	MainPath string
}

var DB *xorm.Engine
var Pool *redis.Pool
var MainPath string
var Vaa3dBin string
var DataPath string
var Tmpdir string
var ImageDir string
var AesKey string

func LoadConfig() error {
	log.SetFormatter(&log.TextFormatter{})
	log.SetOutput(os.Stdout)
	log.SetLevel(log.TraceLevel)

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
	return nil
}

func NewDb(user, passwd, ip, port, db string) error {
	if len(user) == 0 || len(ip) == 0 || len(passwd) == 0 || len(db) == 0 || len(port) == 0 {
		log.WithFields(log.Fields{
			"event": "Allocate DB",
		}).Infoln("Config is Error")
		return errors.New("Config is Error")
	}
	var err error
	DB, err = xorm.NewEngine("mysql", user+":"+passwd+"@tcp("+ip+":"+port+")/"+db)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Allocate DB",
		}).Infof("%v\n", err)
		return err
	}
	DB.ShowSQL(true)

	//DB.SetMaxOpenConns(1000)
	//DB.SetMaxIdleConns(200)
	return nil
}

func NewRedisPool(ip, port string) error {
	if len(ip) == 0 || len(port) == 0 {
		log.WithFields(log.Fields{
			"event": "Allocate Redis Pool",
		}).Infoln("Config is Error")
		return errors.New("Config is Error")
	}

	Pool = &redis.Pool{
		MaxIdle:     5,
		IdleTimeout: 240 * time.Second,
		Dial:        func() (redis.Conn, error) { return redis.Dial("tcp", ip+":"+port) },
	}
	return nil
}
