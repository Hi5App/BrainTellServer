package services

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func Register(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}

	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}
	if len(user.Name) == 0 || len(user.Passwd) == 0 || len(user.NickName) == 0 || len(user.Email) == 0 {
		log.WithFields(log.Fields{
			"event": "Register",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func Login(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if len(user.Passwd) == 0 || (len(user.Name) == 0 && len(user.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	//http.SetCookie(w, &http.Cookie{Name: user.Name, Value: user.Passwd, Expires: time.Now().AddDate(0, 0, 1),
	//	HttpOnly: true, Secure: true, MaxAge: 24 * 3600})
	fmt.Fprintln(w, "Need Implement")
}

func SetUserScore(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if (len(user.Name) == 0 && len(user.Email) == 0) || len(user.Passwd) == 0 || user.Score <= 0 {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func UpdatePasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "UpdatePasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if len(user.Passwd) == 0 || (len(user.Name) == 0 && len(user.Email) == 0) || len(user.NPasswd) == 0 {
		log.WithFields(log.Fields{
			"event": "SetUserScore",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}

	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func ForgetPasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ForgetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if len(user.Email) == 0 {
		log.WithFields(log.Fields{
			"event": "ForgetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}

	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func ResetPasswd(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if len(user.Email) == 0 || len(user.Passwd) == 0 || len(user.NPasswd) == 0 || user.Passwd != user.NPasswd {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")

}

func RegisterNetease(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.UserInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	user, ok := param.(*do.UserInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	if (len(user.Email) == 0 && len(user.Name) == 0) || len(user.Passwd) == 0 || len(user.AppKey) == 0 {
		log.WithFields(log.Fields{
			"event": "ResetPasswd",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")

}
