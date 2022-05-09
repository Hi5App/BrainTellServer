package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	"net/http"

	log "github.com/sirupsen/logrus"
)

type QueryArborDetailParam struct {
	Pa   *do.ArborDetail `json:"pa"`
	User UserVerifyParam `json:"user"`
}

//只需要pa.ArborId

func (pa *QueryArborDetailParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *QueryArborDetailParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func QueryArborDetail(w http.ResponseWriter, r *http.Request) {
	var p QueryArborDetailParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*QueryArborDetailParam)
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

	res, err := ao.QueryArborDetail(&models.TArbordetail{
		Arborid: p.Pa.ArborId,
	})

	if err != nil {
		log.WithFields(log.Fields{
			"event": "QueryArborDetail  Locations",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}
	jsonstr, err := json.Marshal(res)
	if err != nil {
		utils.EncodeToHttp(w, 502, err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, string(jsonstr))
	return
}

type InsertArborDetailParam struct {
	Pa   []*do.ArborDetail `json:"pa"`
	User UserVerifyParam   `json:"user"`
}

func (pa *InsertArborDetailParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *InsertArborDetailParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func InsertArborDetail(w http.ResponseWriter, r *http.Request) {
	var p InsertArborDetailParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*InsertArborDetailParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "InsertArborDetail",
			"desc":  "param.(*do.InsertArborDetailParam) failed",
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

	pp := make([]*models.TArbordetail, 0)
	for _, v := range p.Pa {
		pp = append(pp, &models.TArbordetail{
			Arborid: v.ArborId,
			X:       v.Loc.X,
			Y:       v.Loc.Y,
			Z:       v.Loc.Z,
			Type:    v.Type,
			Owner:   p.User.Name,
		})
	}
	_, err = ao.InsertArborDetail(pp)

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

type DeleteArborDetailParam struct {
	Pa   []int           `json:"pa"`
	User UserVerifyParam `json:"user"`
}

//只需要pa.Id
func (pa *DeleteArborDetailParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *DeleteArborDetailParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func DeleteArbordetail(w http.ResponseWriter, r *http.Request) {
	var p DeleteArborDetailParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*DeleteArborDetailParam)
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

	pp := make([]*models.TArbordetail, 0)
	for _, v := range p.Pa {
		pp = append(pp, &models.TArbordetail{
			Id: v,
		})
	}
	_, err = ao.DeleteArbordetail(pp)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "DeleteArbordetail ",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 501, err.Error())
		return
	}

	utils.EncodeToHttp(w, 200, "")
	return
}
