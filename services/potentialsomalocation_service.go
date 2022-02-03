package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
	"sync"
)

var mutex sync.Mutex
var queue []*do.PotentialSomaLocation

func GetPotentialSomaLocation(w http.ResponseWriter, r *http.Request) {
	var p UserVerifyParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*UserVerifyParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.Name,
		Passwd: p.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	//构建一个队列
	mutex.Lock()
	defer mutex.Unlock()
	if len(queue) == 0 {
		locations, err := ao.GetPotentialSomaLocation()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Get Potential Locations",
				"desc":  "Query MySQL failed",
			}).Warnf("%v\n", err)
			utils.EncodeToHttp(w, 501, err.Error())
			return
		}
		queue = locations
	}

	for i, v := range queue {
		if err := utils.LockLocation(v.Id, p.Name); err != nil {
			continue
		}
		queue = queue[i+1:]
		jsonbody, err := json.Marshal(v)
		if err != nil {
			log.Error(err)
			utils.EncodeToHttp(w, 500, err.Error())
			return
		}
		utils.EncodeToHttp(w, 200, string(jsonbody))
		return

	}
	queue = nil
	utils.EncodeToHttp(w, 502, "Empty")
	return
}
