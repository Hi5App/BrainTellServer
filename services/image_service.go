package services

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type CropBB struct {
	Loc    utils.XYZ `json:"loc"`
	Image  string    `json:"image"`
	RES    string    `json:"res"`
	RESIdx int       `json:"residx"`
}

func (pa *CropBB) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *CropBB) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func GetImageList(w http.ResponseWriter, r *http.Request) {
	//todo
	var p do.Image
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}
	_, ok := param.(*do.Image)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func CropImage(w http.ResponseWriter, r *http.Request) {
	//todo
	var p CropBB
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}

	_, ok := param.(*CropBB)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}

func CropSWC(w http.ResponseWriter, r *http.Request) {
	//todo
	var p CropBB
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		w.WriteHeader(500)
	}

	_, ok := param.(*CropBB)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		w.WriteHeader(500)
	}

	w.WriteHeader(200)
	fmt.Fprintln(w, "Need Implement")
}
