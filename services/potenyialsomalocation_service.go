package services

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
	"sync"
)

var mutex sync.Mutex
var queue []*do.PotentialSomaLocation

func GetPotentialSomaLocation(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.PotentialSomaLocation
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	//构建一个队列
	mutex.Lock()
	defer mutex.Unlock()
	if len(queue) == 0 {
		locataion, ok := param.(*do.PotentialSomaLocation)
		if !ok {
			log.WithFields(log.Fields{
				"event": "GetPotentialSomaLocations",
				"desc":  "param.(*do.PotentialSomaLocation) failed",
			}).Warnf("%v\n", err)
			w.WriteHeader(500)
		}

		if len(locataion.Image) == 0 {
			log.WithFields(log.Fields{
				"event": "GetPotentialSomaLocations",
				"desc":  "Bad Param",
			}).Warnf("%v\n", err)
			w.WriteHeader(400)
		}

		w.WriteHeader(200)
		fmt.Fprintln(w, "Need Implement")
	}

}
