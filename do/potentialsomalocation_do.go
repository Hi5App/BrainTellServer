package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type PotentialSomaLocation struct {
	Id    int64     `json:"id"`
	Image string    `json:"image"`
	Loc   utils.XYZ `json:"loc"`
	Owner string    `json:"owner"`
}

func QueryPotentialSomaLocation(pa *models.TPotentialsomalocation, pd *utils.QueryCondition) ([]*PotentialSomaLocation, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	locations := make([]*models.TPotentialsomalocation, 0)
	//??
	//默认升序
	session := utils.DB.Where("Isdeleted = ?", 0).And("Owner = ?", "").OrderBy("ctime")
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Off)
	}
	err := session.Find(&locations, pa)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query Potentialsomalocation",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*PotentialSomaLocation, 0)
	for _, location := range locations {
		res = append(res, &PotentialSomaLocation{
			Id:    int64(location.Id),
			Image: location.Image,
			Loc: utils.XYZ{
				X: float64(location.X),
				Y: float64(location.Y),
				Z: float64(location.Z),
			},
			Owner: location.Owner,
		})
	}
	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query Potentialsomalocation",
		"pa":    jsonpa,
		"RES":   jsonres,
	}).Infof("Success")
	return res, nil
}

func UpdatePotentialSomaLocation(pa *models.TPotentialsomalocation) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	var pc models.TPotentialsomalocation
	pc = *pa
	pa.Owner = ""
	pa.Type = 0
	//这里不用事务吧，而且事务没有开启，是否应该传入&pc
	affect, err := utils.DB.NewSession().Update(pc, pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Update PotentialSomaLocation",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}
	log.WithFields(log.Fields{
		"event":  "Delete PotentialSomaLocation",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}
