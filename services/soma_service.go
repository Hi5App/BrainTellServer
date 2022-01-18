package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func GetSomaList(w http.ResponseWriter, r *http.Request) {
	var p utils.QuerySomaListParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	qp, ok := param.(*utils.QuerySomaListParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetSomaList",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	if _, err := ao.Login(&qp.User); err != nil {
		utils.EncodeToHttp(w, 401, "")
		return
	}
	if len(qp.Image) == 0 {
		log.WithFields(log.Fields{
			"event": "GetSomaList",
			"desc":  "Bad Request",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	res, err := ao.GetSomaList(&qp.Pa1, &qp.Pa2, qp.Image)
	if err != nil {
		utils.EncodeToHttp(w, 501, "")
		return
	}

	str, err := json.Marshal(res)
	if err != nil {
		utils.EncodeToHttp(w, 502, "")
		return
	}
	utils.EncodeToHttp(w, 200, string(str))
}

func UpdateSomaList(w http.ResponseWriter, r *http.Request) {
	var p utils.UpdateSomaListParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	qp, ok := param.(*utils.UpdateSomaListParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	if _, err := ao.Login(&qp.User); err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	err = ao.UpdateSomaList(qp)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, "")
}
