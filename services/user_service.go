package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type RegisterParam struct {
	Name     string `json:"name"`
	Email    string `json:"email"`
	NickName string `json:"nickname"`
	Passwd   string `json:"passwd"`
}

type GameRegisterParam struct {
	Name   string `json:"name"`
	Email  string `json:"email"`
	Passwd string `json:"passwd"`
}

func (param *RegisterParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *GameRegisterParam) String() string {
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

func (param *GameRegisterParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
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

type LoginParam struct {
	User UserVerifyParam `json:"user"`
}

func (pa *LoginParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *LoginParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func Login(w http.ResponseWriter, r *http.Request) {
	var p LoginParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	fmt.Printf("----------login decode error: %v-------------------\n", err)

	_, ok := param.(*LoginParam)

	fmt.Printf("----------login param transfer error: %v-------------------\n", ok)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(p.User.Passwd) == 0 || (len(p.User.Name) == 0 && len(p.User.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Errorf("%s\n", p)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	userinfo, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Email:  p.User.Email,
		Passwd: p.User.Passwd,
	})

	fmt.Printf("----------login user info: %v-------------------\n", userinfo)
	fmt.Printf("----------login ao.login error: %v-------------------\n", err)

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
	TotalSoma  int64 `json:"totalsoma"`
	DailySoma  int64 `json:"dailysoma"`
	TotalCheck int64 `json:"totalCheck"`
	DailyCheck int64 `json:"dailyCheck"`
}

type GetUserPerformanceParam struct {
	User UserVerifyParam `json:"user"`
}

func (pa *GetUserPerformanceParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetUserPerformanceParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func GetUserPerformance(w http.ResponseWriter, r *http.Request) {
	var p GetUserPerformanceParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	_, ok := param.(*GetUserPerformanceParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if len(p.User.Passwd) == 0 || len(p.User.Name) == 0 {
		log.WithFields(log.Fields{
			"event": "",
			"desc":  "Bad Param",
		}).Warnf("%s\n", p)
		utils.EncodeToHttp(w, 400, "")
		return
	}

	_, err = ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	})
	if err != nil {
		w.WriteHeader(501)
		utils.EncodeToHttp(w, 501, "Verify User failed")
		return
	}

	somaperformance, somadailyperformance, err := ao.GetSomaCnt()
	checkperformance, checkdailyperformance, err := ao.GetCheckCnt()
	if err != nil {
		w.WriteHeader(502)
		utils.EncodeToHttp(w, 502, " failed")
		return
	}

	jsonbody, err := json.Marshal(&UserPerformance{
		TotalSoma:  somaperformance[p.User.Name],
		DailySoma:  somadailyperformance[p.User.Name],
		TotalCheck: checkperformance[p.User.Name],
		DailyCheck: checkdailyperformance[p.User.Name],
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

// GameLogin game func
func GameLogin(w http.ResponseWriter, r *http.Request) {
	var p LoginParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	fmt.Printf("----------login decode error: %v-------------------\n", err)
	_, ok := param.(*LoginParam)
	fmt.Printf("----------login param transfer error: %v-------------------\n", ok)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Game Login",
			"desc":  "Game param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(p.User.Passwd) == 0 || (len(p.User.Name) == 0 && len(p.User.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Game Login",
			"desc":  "Game Bad Param",
		}).Errorf("%s\n", p)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	userinfo, err := ao.GameLogin(&do.GameUserInfo{
		Name:   p.User.Name,
		Email:  p.User.Email,
		Passwd: p.User.Passwd,
	})

	fmt.Printf("----------login user info: %v-------------------\n", userinfo)
	fmt.Printf("----------login ao.login error: %v-------------------\n", err)

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

func GameRegister(w http.ResponseWriter, r *http.Request) {
	var p GameRegisterParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	user, ok := param.(*GameRegisterParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Game Register",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(user.Name) == 0 || len(user.Passwd) == 0 || len(user.Email) == 0 {
		log.WithFields(log.Fields{
			"event": "Game Register",
			"desc":  "Bad Param",
		}).Warnf("%s\n", user)
		utils.EncodeToHttp(w, 400, "Register Failed")
		return
	}

	//todo ao.game register
	err = ao.Register(&do.UserInfo{
		Name:   p.Name,
		Email:  p.Email,
		Passwd: p.Passwd,
	})

	if err != nil {
		utils.EncodeToHttp(w, 501, "Register Failed."+err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, "Register Success")
}
