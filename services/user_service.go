package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func Register(w http.ResponseWriter, r *http.Request) {
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(user.Name) == 0 || len(user.Passwd) == 0 || len(user.NickName) == 0 || len(user.Email) == 0 {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "Register Failed")
		return
	}

	err = ao.Register(user)
	if err != nil {
		utils.EncodeToHttp(w, 501, "Register Failed."+err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, "Register Success")
}

func Login(w http.ResponseWriter, r *http.Request) {
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(user.Passwd) == 0 || (len(user.Name) == 0 && len(user.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	user, err = ao.Login(user)
	if err != nil {
		w.WriteHeader(501)
		utils.EncodeToHttp(w, 501, "Login Failed."+err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, user.String())
	return

}

func SetUserScore(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if (len(user.Name) == 0 && len(user.Email) == 0) || len(user.Passwd) == 0 || user.Score <= 0 {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}
	utils.EncodeToHttp(w, 200, "Need Implement")
}

func UpdatePasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "UpdatePasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if len(user.Passwd) == 0 || (len(user.Name) == 0 && len(user.Email) == 0) || len(user.NPasswd) == 0 {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}

	utils.EncodeToHttp(w, 200, "Need Implement")
}

func ForgetPasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ForgetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if len(user.Email) == 0 {
		log.WithFields(log.Fields{
			"event": "ForgetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}

	utils.EncodeToHttp(w, 200, "Need Implement")
}

func ResetPasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if len(user.Email) == 0 || len(user.Passwd) == 0 || len(user.NPasswd) == 0 || user.Passwd != user.NPasswd {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}
	utils.EncodeToHttp(w, 200, "Need Implement")
}

func RegisterNetease(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if (len(user.Email) == 0 && len(user.Name) == 0) || len(user.Passwd) == 0 || len(user.AppKey) == 0 {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}
	utils.EncodeToHttp(w, 200, "Need Implement")

}

func GetUserPerformance(w http.ResponseWriter, r *http.Request) {
	var p utils.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*utils.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if len(user.Passwd) == 0 || len(user.Name) == 0 {
		log.WithFields(log.Fields{
			"event": "GetUserPerformance",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}
	user, err = ao.Login(user)
	if err != nil {
		w.WriteHeader(501)
		utils.EncodeToHttp(w, 501, "Verify User failed")
		return
	}

	performance, dailyperformance, err := ao.GetUserPerformance()
	if err != nil {
		w.WriteHeader(502)
		utils.EncodeToHttp(w, 502, "GetUserPerformance failed")
		return
	}

	utils.EncodeToHttp(w, 200, fmt.Sprintf("%d\n%d", performance[user.Name], dailyperformance[user.Name]))
}
