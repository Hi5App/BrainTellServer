package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
)

func Register(pa *do.UserInfo) error {
	result := do.InsertUser(&models.UserMetaInfoV1{
		Name:     pa.Name,
		Password: pa.Passwd,
		CompatibleData: models.BrainTellServerMysqlDBCompatibleData{
			Email:     pa.Email,
			NickName:  pa.NickName,
			Score:     0,
			Appkey:    "",
			Isdeleted: 0,
		},
	})
	if !result.Status {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "Insert Failed",
		}).Warnf("%s,%v\n", pa, result.Message)
		return errors.New("DB insert error or User already exist")
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
	userInfo := models.UserMetaInfoV1{}

	result := do.QueryUser(&userInfo, pa.Name)

	fmt.Printf("Login request: UserName: %s , Email: %s", pa.Name, pa.Email)

	if !result.Status {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Login Failed",
		}).Warnf("%s,%v\n", pa, result.Message)
		return nil, errors.New("no such user")
	} else {
		if userInfo.Password != pa.Passwd {
			fmt.Printf("Incorrect Password!")
			return nil, errors.New("incorrect Password")
		}

		// todo redis buffer authentication
		//utils.InsertUser2RDB(users[0])
		return &do.UserInfo{
			Id:       int(userInfo.UserId),
			Name:     userInfo.Name,
			Email:    userInfo.CompatibleData.Email,
			NickName: userInfo.CompatibleData.NickName,
			Score:    userInfo.CompatibleData.Score,
			AppKey:   userInfo.CompatibleData.Appkey,
			Passwd:   userInfo.Password,
		}, nil
	}
}

// GameLogin game func
func GameLogin(pa *do.GameUserInfo) (*do.GameUserInfo, error) {
	// todo redis buffer authentication
	//user, err := utils.QueryUserFromRDB(pa)
	//fmt.Printf("----------user_ao Redis Query user: %v-------------------\n", user)
	//fmt.Printf("----------user_ao Redis Query error: %v-------------------\n", err)
	//
	//if err == nil {
	//	utils.InsertUser2RDB(user)
	//	return user, nil
	//}

	//users, err := do.QueryGameUser(&models.TGameUserinfo{
	//	Name:   pa.Name,
	//	Email:  pa.Email,
	//	Passwd: pa.Passwd,
	//}, &utils.QueryCondition{
	//	Limit: 1, Off: 0,
	//})
	//
	//fmt.Printf("----------user_ao QueryUser users: %v-------------------\n", users)
	//fmt.Printf("----------user_ao QueryUser err: %v-------------------\n", err)
	//
	//if err != nil {
	//	log.WithFields(log.Fields{
	//		"event": "Login",
	//		"desc":  "Game Login Failed",
	//	}).Warnf("%s,%v\n", pa, err)
	//	return nil, err
	//}
	//
	//if len(users) != 0 {
	//	// todo redis buffer authentication
	//	//utils.InsertUser2RDB(users[0])
	//	return users[0], nil
	//}
	//return nil, errors.New("no such user")

	userInfo := models.UserMetaInfoV1{}

	result := do.QueryUser(&userInfo, pa.Name)

	fmt.Printf("Login request: UserName: %s , Email: %s", pa.Name, pa.Email)

	if !result.Status {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Login Failed",
		}).Warnf("%s,%v\n", pa, result.Message)
		return nil, errors.New("no such user")
	} else {
		// todo redis buffer authentication
		//utils.InsertUser2RDB(users[0])
		return &do.GameUserInfo{
			Id:     int(userInfo.UserId),
			Name:   userInfo.Name,
			Email:  userInfo.CompatibleData.Email,
			Score:  userInfo.CompatibleData.Score,
			Passwd: userInfo.Password,
		}, nil
	}
}
