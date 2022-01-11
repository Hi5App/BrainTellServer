package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
	"net/http"
	"sync"
)

var mutex sync.Mutex
var queue []*utils.PotentialSomaLocation

func GetPotentialSomaLocation(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.PotentialSomaLocation
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	locataion, ok := param.(*utils.PotentialSomaLocation)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	if _, err := ao.Login(&locataion.User); err != nil {
		utils.EncodeToHttp(w, 401, "")
		return
	}
	//构建一个队列
	mutex.Lock()
	defer mutex.Unlock()
	if len(queue) == 0 {

		locations, err := ao.GetPotentialSomaLocation(locataion)
		if err != nil {
			log.WithFields(log.Fields{
				"event": "GetPotentialSomaLocations",
				"desc":  "Query MySQL failed",
			}).Warnf("%v\n", err)
			utils.EncodeToHttp(w, 501, "")
			return
		}
		queue = locations
	}

	for i, v := range queue {
		if err := utils.LockLocation(v); err != nil {
			continue
		}
		queue = queue[i+1:]
		utils.EncodeToHttp(w, 200, v.String())
		return
	}
	queue = nil
	utils.EncodeToHttp(w, 502, "")
	return
}
