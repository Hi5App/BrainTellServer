package utils

import (
	"encoding/json"
	"errors"
	"fmt"
	"github.com/gomodule/redigo/redis"
	log "github.com/sirupsen/logrus"
	"strconv"
	"strings"
)

func LockLocation(locationId int64, user string) error {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}

	res, err := redis.Int64(conn.Do("EXISTS", "PotentialSomaLocation"+fmt.Sprint(locationId)))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock",
			"pa":    locationId,
		}).Warnf("%v\n", err)
		return err
	}

	if res != 0 {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock exist",
			"pa":    locationId,
		}).Warnf("%v\n", err)
		return errors.New("PotentialSomaLocation" + fmt.Sprint(locationId) + "Lock exist")
	}

	_, err = redis.String(conn.Do("SETEX", "PotentialSomaLocation"+fmt.Sprint(locationId), 10*60, user))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock exist",
			"pa":    locationId,
		}).Warnf("%v\n", err)
		return errors.New("PotentialSomaLocation" + fmt.Sprint(locationId) + "lock Failed")
	}
	return nil
}

func GetLocationTTL(key string) (int64, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return 0, err
	}
	res, err := redis.Int64(conn.Do("TTL", key))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "GetLocationTTL failed",
		}).Warnf("%v\n", err)
		return 0, err
	}
	if res <= 30 {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "GetLocationTTL <=30",
		}).Warnf("%v\n", err)
		return 0, errors.New(fmt.Sprintf("GetLocationTTL %s <=30", key))
	}
	return res, nil
}

func GetLocationValue(key string) (string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	res, err := redis.String(conn.Do("GET", key))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "GetLocationValue failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	return res, nil
}

func SetKeyTTL(key string, ttl int) error {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}

	res, err := redis.Int64(conn.Do("Expire", key, ttl))
	if err != nil || res == 0 {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "SetKeyTTL",
			"pa":    key,
		}).Warnf("%v\n", err)
		return errors.New(fmt.Sprintf("%s set ttl failed", key))
	}

	return nil
}

// QueryUserFromRDB 查询用户信息
func QueryUserFromRDB(pa *UserInfo) (*UserInfo, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return nil, err
	}

	var res string
	var err error
	if len(pa.Name) != 0 {
		res, err = redis.String(conn.Do("Get", "USERNAME_"+pa.Name))
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Redis",
				"desc":  "Get USERNAME_" + pa.Name + " failed",
			}).Warnf("%v\n", err)
			return nil, err
		}
	} else if len(pa.Email) == 0 {
		res, err = redis.String(conn.Do("Get", "USEREMAIL_"+pa.Email))
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Redis",
				"desc":  "Get USERNAME_" + pa.Name + " failed",
			}).Warnf("%v\n", err)
			return nil, err
		}
	}

	err = json.Unmarshal([]byte(res), pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  " FromJsonString Failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return pa, nil
}

// InsertUser2RDB 插入用户信息到redis
func InsertUser2RDB(pa *UserInfo) error {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}

	if len(pa.Name) == 0 && len(pa.Email) == 0 {
		return errors.New("pa is empty")
	}
	jsonbody, err := json.Marshal(pa)
	if err != nil {
		return err
	}
	conn.Do("SETEX", "USERNAME_"+pa.Name, 600, string(jsonbody))
	conn.Do("SETEX", "USEREMAIL_"+pa.Email, 600, string(jsonbody))
	return nil
}

// InsertImage2RDB 插入图像信息到redis
func InsertImage2RDB(pa []*Image) error {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}
	if len(pa) == 0 {
		return errors.New("empty ")
	}
	str, err := json.Marshal(pa)
	if err != nil {
		return err
	}
	conn.Do("SETEX", "ImageList", 5*60, str)
	return nil
}

// GetImageFromRDB 从redis获取图像信息
func GetImageFromRDB() (string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	res, err := redis.String(conn.Do("Get", "ImageList"))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get ImageList failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	return res, nil
}

// GetMusicListFromRDB 从redis获取音乐名称
func GetMusicListFromRDB() (string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	res, err := redis.Strings(conn.Do("LRANGE", "MUSICLIST", 0, -1))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get ImageList failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	str, err := json.Marshal(res)
	if err != nil {
		return "", err
	}
	return string(str), nil
}

// GetLastestApkRes 查询最新的apk版本
func GetLastestApkRes() ([]string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	res, err := redis.Strings(conn.Do("LRANGE", "ApkVersion", 0, -1))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get ApkVersion failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return res, nil
}

// InsertPerformance2RDB 将用户点的soma个数插入到redis
func InsertPerformance2RDB(key string, values map[string]int64) error {
	if len(values) == 0 {
		return nil
	}
	var value []interface{}
	value = append(value, key)
	for k, v := range values {
		value = append(value, k)
		value = append(value, fmt.Sprint(v))
	}

	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}

	_, err := redis.Int64(conn.Do("RPUSH", value...))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "InsertPerformance2RDB",
		}).Warnf("%v\n", err)
		return err
	}
	conn.Do("EXPIRE", key, 60*60)
	return nil
}

// QueryPerformance2RDB 从redis查询用户点的soma
func QueryPerformance2RDB(key string) (map[string]int64, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return nil, err
	}

	exist, err := redis.Int64(conn.Do("EXISTS", key))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "QueryPerformance2RDB EXISTS",
		}).Warnf("%v\n", err)
		return nil, err
	}

	if exist != 1 {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "QueryPerformance2RDB EXISTS",
		}).Infof("%s not exist", key)
		return nil, errors.New("not exist")
	}

	values, err := redis.Strings(conn.Do("LRANGE", key, 0, -1))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "QueryPerformance2RDB",
		}).Warnf("%v\n", err)
		return nil, err
	}
	res := make(map[string]int64)

	if len(values)%2 != 0 {
		return nil, errors.New("failed ")
	}

	for i := 0; i < len(values); i += 2 {
		v, _ := strconv.Atoi(values[i+1])
		res[values[i]] = int64(v)
	}
	return res, nil
}

func AllocatePort(ano string) (string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return "", err
	}

	port, err := redis.String(conn.Do("LPOP", "PORTQUEUE"))
	if err != nil {
		return "", err
	}

	res, err := redis.Int(conn.Do("EXISTS", fmt.Sprintf("Ano+Port:%s;%s", "*", port)))
	if err != nil {
		conn.Do("RPUSH", "PORTQUEUE", port)
		return "", err
	}

	if res == 1 {
		conn.Do("RPUSH", "PORTQUEUE", port)
		log.Error("Port %d Has in Use", port)
		return "-1", errors.New("port Has in Use，please try again")
	}

	ret, err := redis.String(conn.Do("SETEX", fmt.Sprintf("Ano+Port:%s;%s", ano, port), 3*10*60, ano))
	if err != nil {
		return "", err
	}
	if ret != "OK" {
		return "", errors.New("can not allocate port")
	}
	return port, nil
}

func QueryAnoPort(ano string) (string, error) {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return "", err
	}

	keys, err := redis.Strings(conn.Do("Keys", fmt.Sprintf("Ano+Port:%s;%s", ano, "*")))
	if err != nil {
		//error
		return "", err
	}
	if len(keys) == 0 {
		//not exist
		return "", nil
	}

	if len(keys) != 1 {
		//存在多个，错误
		return "", errors.New("multiply port exist")
	} else {
		//存在，取出端口号，key->(ano,port)
		var anowithport = ""
		var port = ""
		_, err := fmt.Sscanf(keys[0], "Ano+Port:%s", &anowithport)
		if err != nil {
			return "", err
		}
		port = strings.Split(anowithport, ";")[1]
		return port, nil
	}

}
