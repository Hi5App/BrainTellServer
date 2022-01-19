package services

import (
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func GetMusicList(w http.ResponseWriter, r *http.Request) {
	res, err := utils.GetMusicListFromRDB()
	if err != nil {
		w.WriteHeader(500)
		return
	}
	utils.EncodeToHttp(w, 200, res)
}

func GetLatestApk(w http.ResponseWriter, r *http.Request) {
	var p utils.GetLatestApkParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	qp, ok := param.(*utils.GetLatestApkParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	apkinfo, err := utils.GetLastestApkRes()
	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetLastestApkRes",
			"desc":  "GetLastestApkRes Failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}

	if apkinfo[0] != qp.Version {
		utils.EncodeToHttp(w, 200, apkinfo[1])
		return
	}
	utils.EncodeToHttp(w, 201, "Your veriosn is lastest")
}
