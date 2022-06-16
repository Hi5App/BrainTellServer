package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
)

func Register(pa *do.UserInfo) error {
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

func Login(pa *do.UserInfo) (*do.UserInfo, error) {
	// todo redis buffer authentication
	//user, err := utils.QueryUserFromRDB(pa)
	//fmt.Printf("----------user_ao Redis Query user: %v-------------------\n", user)
	//fmt.Printf("----------user_ao Redis Query error: %v-------------------\n", err)
	//
	//if err == nil {
	//	utils.InsertUser2RDB(user)
	//	return user, nil
	//}

	users, err := do.QueryUser(&models.TUserinfo{
		Name:   pa.Name,
		Email:  pa.Email,
		Passwd: pa.Passwd,
	}, &utils.QueryCondition{
		Limit: 1, Off: 0,
	})

	fmt.Printf("----------user_ao QueryUser users: %v-------------------\n", users)
	fmt.Printf("----------user_ao QueryUser err: %v-------------------\n", err)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Login Failed",
		}).Warnf("%s,%v\n", pa, err)
		return nil, err
	}

	if len(users) != 0 {
		// todo redis buffer authentication
		//utils.InsertUser2RDB(users[0])
		return users[0], nil
	}
	return nil, errors.New("no such user")
}
