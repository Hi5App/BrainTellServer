package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type UserInfo struct {
	Name     string `json:"name"`
	Email    string `json:"email"`
	NickName string `json:"nickname"`
	Score    int    `json:"score"`
	AppKey   string `json:"appkey"`
	Passwd   string `json:"passwd"`
	NPasswd  string `json:"npasswd"`
}

func (user *UserInfo) String() string {
	jsonres, err := json.Marshal(user)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (user *UserInfo) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), user); err != nil {
		log.WithFields(log.Fields{
			"event": "Query userinfo",
			"pa":    jsonstr,
		}).Warnf("%s\n%v\n", string([]byte(jsonstr)), err)
		return nil, err
	}
	return user, nil
}

func (user *UserInfo) Error() string {
	return "Error"
}

func QueryUser(pa *models.TUserinfo, pd *utils.QueryCondition) ([]*UserInfo, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	users := make([]models.TUserinfo, 0)
	session := utils.DB.Where("Isdeleted = ?", 0)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Length)
	}
	err := session.Find(users, pa)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query userinfo",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*UserInfo, 0)
	for _, user := range users {
		res = append(res, &UserInfo{
			Name:     user.Name,
			Email:    user.Email,
			Score:    user.Score,
			AppKey:   user.Appkey,
			NickName: user.Nickname,
		})
	}

	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query userinfo",
		"pa":    jsonpa,
		"RES":   jsonres,
	}).Infof("Success")

	return res, nil
}

func InsertUser(pa *models.TUserinfo) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	affect, err := utils.DB.Insert(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Insert userinfo",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Insert userinfo",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}

func UpdateUser(pa *models.TUserinfo, pc *models.TUserinfo) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	affect, err := utils.DB.Update(pa, pc)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Update userinfo",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Update userinfo",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}

func DeltelUser(pa *models.TUserinfo) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	var pc *models.TUserinfo
	*pc = *pa
	pa.Isdeleted = 1
	affect, err := utils.DB.Update(pa, pc)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Delete userinfo",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Delete userinfo",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}
