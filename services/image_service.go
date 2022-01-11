package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func GetImageList(w http.ResponseWriter, r *http.Request) {
	var p utils.Image
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	qp, ok := param.(*utils.Image)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	if _, err := ao.Login(&qp.User); err != nil {
		utils.EncodeToHttp(w, 401, "")
		return
	}
	str, err := ao.GetImageList()
	if err != nil {
		utils.EncodeToHttp(w, 501, "")
		return
	}
	utils.EncodeToHttp(w, 200, str)
}

func CropImage(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.CropBB
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}
	pa, ok := param.(*utils.CropBB)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	if _, err := ao.Login(&pa.User); err != nil {
		utils.EncodeToHttp(w, 401, "")
		return
	}
	out, err := utils.GetBB(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, "")
		return
	}

	utils.SendFile(w, 200, out)

}

func CropSWC(w http.ResponseWriter, r *http.Request) {
	//todo
	var p utils.CropBB
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, "")
		return
	}

	_, ok := param.(*utils.CropBB)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}
	utils.EncodeToHttp(w, 200, "Need Implement")
}
