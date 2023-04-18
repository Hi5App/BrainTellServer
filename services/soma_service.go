package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func GetSomaList(w http.ResponseWriter, r *http.Request) {
	var p BBoxParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*BBoxParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetSomaList",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	if len(p.BB.Obj) == 0 {
		utils.EncodeToHttp(w, 400, "Bad Request")
	}
	res, err := ao.GetSomaList(&p.BB.Pa1, &p.BB.Pa2, p.BB.Obj)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}

	str, err := json.Marshal(res)
	if err != nil {
		utils.EncodeToHttp(w, 502, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, string(str))
}

type UpdateSomaParam struct {
	Pa   ao.UpdateSomaAo `json:"pa"`
	User UserVerifyParam `json:"user"`
}

func (param *UpdateSomaParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *UpdateSomaParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func UpdateSomaList(w http.ResponseWriter, r *http.Request) {
	var p UpdateSomaParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	_, ok := param.(*UpdateSomaParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			//"desc":  "param.(*UpdataSomaParam) failed",
			"desc": "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}
	p.Pa.Owner = p.User.Name
	err = ao.UpdateSomaList(&p.Pa)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, "")
}
