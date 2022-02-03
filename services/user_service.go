package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type RegisterParam struct {
	Name     string `json:"name"`
	Email    string `json:"email"`
	NickName string `json:"nickname"`
	Passwd   string `json:"passwd"`
}

func (param *RegisterParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *RegisterParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func Register(w http.ResponseWriter, r *http.Request) {
	var p RegisterParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	user, ok := param.(*RegisterParam)
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

	err = ao.Register(&do.UserInfo{
		Name:     p.Name,
		Email:    p.Email,
		NickName: p.NickName,
		Passwd:   p.Passwd,
	})

	if err != nil {
		utils.EncodeToHttp(w, 501, "Register Failed."+err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, "Register Success")
}

type UserVerifyParam struct {
	Name   string `json:"name"`
	Email  string `json:"email"`
	Passwd string `json:"passwd"`
}

func (param *UserVerifyParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *UserVerifyParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func Login(w http.ResponseWriter, r *http.Request) {
	var p UserVerifyParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	user, ok := param.(*UserVerifyParam)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(user.Passwd) == 0 || (len(user.Name) == 0 && len(user.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Errorf("%s\n", user)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	userinfo, err := ao.Login(&do.UserInfo{
		Name:   p.Name,
		Email:  p.Email,
		Passwd: p.Passwd,
	})
	if err != nil {
		w.WriteHeader(501)
		utils.EncodeToHttp(w, 501, "Login Failed."+err.Error())
		return
	}
	jsonbody, err := json.Marshal(userinfo)
	if err != nil {
		log.Error(err)
		return
	}
	utils.EncodeToHttp(w, 200, string(jsonbody))
	return

}

type UserPerformance struct {
	TotalSoma int64 `json:"totalsoma"`
	DailySoma int64 `json:"dailysoma"`
}

func GetUserPerformance(w http.ResponseWriter, r *http.Request) {
	var p UserVerifyParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	user, ok := param.(*UserVerifyParam)
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
			"event": "",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "")
		return
	}

	_, err = ao.Login(&do.UserInfo{
		Name:   p.Name,
		Passwd: p.Passwd,
	})
	if err != nil {
		w.WriteHeader(501)
		utils.EncodeToHttp(w, 501, "Verify User failed")
		return
	}

	performance, dailyperformance, err := ao.GetSomaCnt()
	if err != nil {
		w.WriteHeader(502)
		utils.EncodeToHttp(w, 502, " failed")
		return
	}
	jsonbody, err := json.Marshal(&UserPerformance{
		TotalSoma: performance[user.Name],
		DailySoma: dailyperformance[user.Name],
	})
	if err != nil {
		log.Error(err)
	}
	utils.EncodeToHttp(w, 200, string(jsonbody))
}

func SetUserScore(w http.ResponseWriter, r *http.Request) {
	//todo
}

func UpdatePasswd(w http.ResponseWriter, r *http.Request) {
	//todo
}

func ResetPasswd(w http.ResponseWriter, r *http.Request) {
	//todo
}

func RegisterNetease(w http.ResponseWriter, r *http.Request) {
	//todo
}
