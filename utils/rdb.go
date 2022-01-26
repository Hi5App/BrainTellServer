package utils

import (
	"encoding/json"
	"errors"
	"fmt"
	"github.com/gomodule/redigo/redis"
	log "github.com/sirupsen/logrus"
	"strconv"
)

func LockLocation(location *PotentialSomaLocation, user string) error {
	conn := Pool.Get()
	defer conn.Close()
	if _, err := conn.Do("SELECT", 0); err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Get conn failed",
		}).Warnf("%v\n", err)
		return err
	}

	res, err := redis.Int64(conn.Do("EXISTS", "PotentialSomaLocation"+fmt.Sprint(location.Id)))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock",
			"pa":    location,
		}).Warnf("%v\n", err)
		return err
	}

	if res != 0 {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock exist",
			"pa":    location,
		}).Warnf("%v\n", err)
		return errors.New("PotentialSomaLocation" + fmt.Sprint(location.Id) + "Lock exist")
	}

	_, err = redis.String(conn.Do("SETEX", "PotentialSomaLocation"+fmt.Sprint(location.Id), 10*60, user))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  "Set Lock exist",
			"pa":    location,
		}).Warnf("%v\n", err)
		return errors.New("PotentialSomaLocation" + fmt.Sprint(location.Id) + "lock Failed")
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
	param, err := pa.FromJsonString(res)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  " FromJsonString Failed",
		}).Warnf("%v\n", err)
		return nil, err
	}

	pa, ok := param.(*UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Redis",
			"desc":  " param.(*do.UserInfo) Failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return pa, nil
}

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
	conn.Do("SETEX", "USERNAME_"+pa.Name, 60, pa.String())
	conn.Do("SETEX", "USEREMAIL_"+pa.Email, 60, pa.String())

	return nil
}

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

func InsertPerformance2RDB(key string, values map[string]int64) error {
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
