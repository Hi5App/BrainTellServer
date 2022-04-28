package do

import (
	"BrainTellServer/utils"
	"fmt"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
	"strconv"
)

type Arbor struct {
	Id     int       `json:"id"`
	Name   string    `json:"name"`
	SomaId string    `json:"somaId"`
	Image  string    `json:"image"`
	Loc    utils.XYZ `json:"loc"`
	Status int       `json:"status"`
}

func QueryArbors(owner string) ([]*Arbor, error) {
	sql := "select * from t_arbor where Isdeleted=0 " +
		"and Id not in (select ArborId from t_arborresult where t_arborresult.Isdeleted=0 and Owner= " +
		fmt.Sprintf("\"%s\"", owner) + ")" +
		"and Id not in (select ArborId from t_arborresult where t_arborresult.Isdeleted=0 group by ArborId having count(*) >10) limit 10"

	arbors, err := utils.DB.QueryString(sql)
	if err != nil {
		return nil, err
	}
	res := make([]*Arbor, 0)

	for _, arbor := range arbors {
		var val Arbor

		if v, err := strconv.Atoi(arbor["Id"]); err != nil {
			continue
		} else {
			val.Id = v
		}

		if v, ok := arbor["Name"]; !ok {
			continue
		} else {
			val.Name = v
		}

		if v, ok := arbor["SomaId"]; !ok {
			continue
		} else {
			val.SomaId = v
		}

		if v, ok := arbor["Image"]; !ok {
			continue
		} else {
			val.Image = v
		}

		if v, err := strconv.ParseFloat(arbor["X"], 64); err != nil {
			continue
		} else {
			val.Loc.X = v
		}

		if v, err := strconv.ParseFloat(arbor["Y"], 64); err != nil {
			continue
		} else {
			val.Loc.Y = v
		}

		if v, err := strconv.ParseFloat(arbor["Z"], 64); err != nil {
			continue
		} else {
			val.Loc.Z = v
		}

		if v, err := strconv.Atoi(arbor["Status"]); err != nil {
			continue
		} else {
			val.Status = v
		}

		res = append(res, &val)
	}

	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query Arbor",
		"RES":   jsonres,
		"Owner": owner,
	}).Infof("Success")
	return res, nil
}
