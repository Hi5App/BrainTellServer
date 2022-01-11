package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

func QueryPotentialSomaLocation(pa *models.TPotentialsomalocation, pd *utils.QueryCondition) ([]*utils.PotentialSomaLocation, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	locations := make([]*models.TPotentialsomalocation, 0)
	session := utils.DB.Where("Isdeleted = ?", 0)
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

	res := make([]*utils.PotentialSomaLocation, 0)
	for _, location := range locations {
		res = append(res, &utils.PotentialSomaLocation{
			Id:    int64(location.Id),
			Image: location.Image,
			Loc: utils.XYZ{
				X: location.X,
				Y: location.Y,
				Z: location.Y,
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
