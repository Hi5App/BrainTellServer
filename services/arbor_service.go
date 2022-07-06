package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

type GetArborParam struct {
	User UserVerifyParam `json:"user"`
	// prevent repeated arborId
	MaxId int64 `json:"maxId"`
}

func (pa *GetArborParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetArborParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func GetArbor(w http.ResponseWriter, r *http.Request) {
	var p GetArborParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*GetArborParam)
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

	locations, err := ao.GetArbors(p.User.Name, p.MaxId)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Get Potential Locations",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	jsonstr, err := json.Marshal(locations)
	if err != nil {
		utils.EncodeToHttp(w, 502, err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, string(jsonstr))
}
