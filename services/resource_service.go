package services

import (
	"BrainTellServer/utils"
	"encoding/json"
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

type GetLatestApkParam struct {
	Version string `json:"version"`
}

func (pa *GetLatestApkParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetLatestApkParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		log.WithFields(log.Fields{
			"event": "Query userinfo",
			"pa":    jsonstr,
		}).Warnf("%s\n%v\n", string([]byte(jsonstr)), err)
		return nil, err
	}
	return pa, nil
}

func GetLatestApk(w http.ResponseWriter, r *http.Request) {
	var p GetLatestApkParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	qp, ok := param.(*GetLatestApkParam)
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
