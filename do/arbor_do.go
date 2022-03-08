package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type Arbor struct {
	Id     int       `json:"id"`
	Name   string    `json:"name"`
	SomaId string    `json:"somaId"`
	Image  string    `json:"image"`
	Loc    utils.XYZ `json:"loc"`
	Status int       `json:"status"`
	Owner  string    `json:"owner"`
}

func QueryArbors(pa *models.TArbor, pd *utils.QueryCondition) ([]*Arbor, error) {
	arbors := make([]*models.TArbor, 0)
	session := utils.DB.Where("Isdeleted = ?", 0).And("Owner = ?", "").OrderBy("ctime")
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Off)
	}
	err := session.Find(&arbors, pa)

	jsonpa, _ := jsoniter.MarshalToString(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query Arbor",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*Arbor, 0)
	for _, arbor := range arbors {
		res = append(res, &Arbor{
			Id:     arbor.Id,
			Name:   arbor.Name,
			SomaId: arbor.Somaid,
			Image:  arbor.Image,
			Loc: utils.XYZ{
				X: arbor.X,
				Y: arbor.Y,
				Z: arbor.Z,
			},
			Status: arbor.Status,
			Owner:  arbor.Owner,
		})
	}
	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query Arbor",
		"pa":    jsonpa,
		"RES":   jsonres,
	}).Infof("Success")
	return res, nil
}

func UpdateArbor(pa *models.TArbor) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)
	var pc models.TArbor
	pc = *pa
	pa.Owner = ""
	affect, err := utils.DB.NewSession().Update(pc, pa)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Update Arbor",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}
	log.WithFields(log.Fields{
		"event":  "Update Arbor",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}
