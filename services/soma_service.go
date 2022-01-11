package services

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func GetSomaList(w http.ResponseWriter, r *http.Request) {
	var p do.SomaInfo
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	qp, ok := param.(*do.SomaInfo)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}
	if len(qp.Image) == 0 {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Request",
		}).Warnf("%v\n", err)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func InsertSomaList(w http.ResponseWriter, r *http.Request) {
	var p do.QuerySomaListParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	qp, ok := param.(*do.QuerySomaListParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}
	if len(qp.Image) == 0 {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Request",
		}).Warnf("%v\n", err)
		w.WriteHeader(400)
	}
	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}
