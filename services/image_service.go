package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type Image struct {
	do.Image
	User UserVerifyParam `json:"user"`
}

func (image *Image) String() string {
	jsonres, err := json.Marshal(image)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (image *Image) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), image); err != nil {
		return nil, err
	}
	return image, nil
}

func GetImageList(w http.ResponseWriter, r *http.Request) {
	var p Image
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*Image)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	str, err := ao.GetImageList(&p.Image)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, str)
}

type BBoxParam struct {
	BB   utils.BBox      `json:"bb"`
	User UserVerifyParam `json:"user"`
}

func (param *BBoxParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *BBoxParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func CropImage(w http.ResponseWriter, r *http.Request) {
	var p BBoxParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*BBoxParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	out, err := utils.GetBBImage(&p.BB)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.SendFile(w, 200, out)
}

func CropSWC(w http.ResponseWriter, r *http.Request) {
	var p BBoxParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*BBoxParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "CropImage",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, "")
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	out, err := utils.GetBBSwc(&p.BB)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "CropSwc",
			"desc":  "param.(*CropImage) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	utils.SendFile(w, 200, out)
}

func CropApo(w http.ResponseWriter, r *http.Request) {
	//todo
}
