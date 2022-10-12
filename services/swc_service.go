package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type InsertBranchingPointsParam struct {
	//Sd   []*do.SwcDetail `json:"sd"`
	Sd   do.SwcDetail    `json:"sd"`
	User UserVerifyParam `json:"user"`
}

func (i *InsertBranchingPointsParam) String() string {
	jsonres, err := json.Marshal(i)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (i *InsertBranchingPointsParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), i); err != nil {
		return nil, err
	}
	return i, nil
}

func InsertBranchingPoints(w http.ResponseWriter, r *http.Request) {

	var p InsertBranchingPointsParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*InsertBranchingPointsParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "InsertBranchingPoints",
			"desc":  "param.(*do.InsertBranchingPointsParam) failed",
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

	//pp := make([]*models.TGameRecord, 0)
	//for _, v := range p.Sd {
	//	pp = append(pp, &models.TGameRecord{
	//		Swcid:     v.Swcid,
	//		Username:  v.Username,
	//		Points:    v.Points,
	//		Correctbp: v.Correctbp,
	//		Wrongbp:   v.Wrongbp,
	//		Missedbp:  v.Missedbp,
	//	})
	//}

	err = ao.InsertBP(&models.TGameRecord{
		Swcid:     p.Sd.Swcid,
		Username:  p.User.Name,
		Points:    p.Sd.Points,
		Correctbp: p.Sd.Correctbp,
		Wrongbp:   p.Sd.Wrongbp,
		Missedbp:  p.Sd.Missedbp,
	})

	if err != nil {
		log.WithFields(log.Fields{
			"event": "InsertArborDetail  Locations",
			"desc":  "Insert MySQL failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, "")
	return

}
