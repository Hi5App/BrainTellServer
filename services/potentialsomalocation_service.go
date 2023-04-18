package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
	"sync"
)

var PotentialMutex sync.Mutex
var PotentialQueue []*do.PotentialSomaLocation

type GetPotentialSomaLocationParam struct {
	User UserVerifyParam `json:"user"`
}

func (pa *GetPotentialSomaLocationParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetPotentialSomaLocationParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

// ??
// 一次只能获取一个
func GetPotentialSomaLocation(w http.ResponseWriter, r *http.Request) {
	var p GetPotentialSomaLocationParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*GetPotentialSomaLocationParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
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

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	//构建一个队列
	PotentialMutex.Lock()
	defer PotentialMutex.Unlock()
	if len(PotentialQueue) == 0 {
		locations, err := ao.GetPotentialSomaLocation()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Get Potential Locations",
				"desc":  "Query MySQL failed",
			}).Warnf("%v\n", err)
			utils.EncodeToHttp(w, 501, err.Error())
			return
		}
		PotentialQueue = locations
	}

	for i, v := range PotentialQueue {
		if err := utils.LockLocation("PotentialSomaLocation"+fmt.Sprint(v.Id), p.User.Name, 60*10, true); err != nil {
			continue
		}

		PotentialQueue = PotentialQueue[i+1:]
		jsonbody, err := json.Marshal(v)
		if err != nil {
			log.Error(err)
			utils.EncodeToHttp(w, 500, err.Error())
			return
		}
		utils.EncodeToHttp(w, 200, string(jsonbody))
		return

	}
	PotentialQueue = nil
	utils.EncodeToHttp(w, 502, "Empty")
	return
}
