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

func InsertSomaList(w http.ResponseWriter, r *http.Request) {
	var p utils.InsertSomaListParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	qp, ok := param.(*utils.InsertSomaListParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	if _, err := ao.Login(&qp.User); err != nil {
		utils.EncodeToHttp(w, 401, "")
		return
	}
	if len(qp.Somalist) == 0 {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Request",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 400, "")
		return
	}
	err = ao.InsertSomaList(qp)
	if err != nil {
		utils.EncodeToHttp(w, 501, "")
		return
	}
	utils.EncodeToHttp(w, 200, "")
}
