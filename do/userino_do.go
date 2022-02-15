package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type UserInfo = utils.UserInfo

func QueryUser(pa *models.TUserinfo, pd *utils.QueryCondition) ([]*UserInfo, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	users := make([]*models.TUserinfo, 0)
	session := utils.DB.NewSession()
	defer session.Close()
	session.Where("Isdeleted = ?", 0)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Off)
	}
	err := session.Find(&users, &models.TUserinfo{

		Name:   pa.Name,
		Email:  pa.Email,
		Passwd: pa.Passwd,
	})

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
			Id:       user.Id,
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
	session := utils.DB.NewSession()
	defer session.Close()
	affect, err := session.Insert(pa)
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

	affect, err := utils.DB.NewSession().Update(pa, pc)
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
	affect, err := utils.DB.NewSession().Update(pa, pc)
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
