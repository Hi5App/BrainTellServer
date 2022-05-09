package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type UpdateArborResultParam struct {
	Pa   ao.UpdateArboResultAo `json:"pa"`
	User UserVerifyParam       `json:"user"`
}

func (param *UpdateArborResultParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *UpdateArborResultParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func UpdateArborResult(w http.ResponseWriter, r *http.Request) {
	var p UpdateArborResultParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*UpdateArborResultParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
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

	for _, pv := range p.Pa.Insertlist {
		pv.Owner = p.User.Name
	}

	err = ao.UpdateArborResult(&p.Pa)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, "")
}

type QueryArborResultParam struct {
	ArborId int             `json:"arborId"`
	User    UserVerifyParam `json:"user"`
}

func (param *QueryArborResultParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *QueryArborResultParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func QueryArborResult(w http.ResponseWriter, r *http.Request) {
	var p QueryArborResultParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*QueryArborResultParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
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

	res, err := ao.QueryArborResult(p.ArborId)
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
