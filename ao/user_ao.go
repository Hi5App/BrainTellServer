package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
)

func Register(pa *utils.UserInfo) error {
	_, err := do.InsertUser(&models.TUserinfo{
		Name:     pa.Name,
		Email:    pa.Email,
		Passwd:   pa.Passwd,
		Nickname: pa.NickName,
	})
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "Insert Failed",
		}).Warnf("%s,%v\n", pa, err)
		return err
	}
	return nil
}

func Login(pa *utils.UserInfo) (*utils.UserInfo, error) {
	user, err := utils.QueryUserFromRDB(pa)
	if err == nil {
		utils.InsertUser2RDB(user)
		return user, nil
	}
	users, err := do.QueryUser(&models.TUserinfo{
		Name:   pa.Name,
		Email:  pa.Email,
		Passwd: pa.Passwd,
	}, &utils.QueryCondition{
		Limit: 1, Off: 0,
	})
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Login Failed",
		}).Warnf("%s,%v\n", pa, err)
		return nil, err
	}
	if len(users) != 0 {
		return users[0], nil
	}
	utils.InsertUser2RDB(users[0])
	return nil, nil
}
