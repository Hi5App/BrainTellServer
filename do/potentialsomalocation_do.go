package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type PotentialSomaLocation struct {
	Image     string               `json:"image"`
	Loc       utils.XYZ            `json:"loc"`
	Owner     string               `json:"owner"`
	User      UserInfo             `json:"user"`
	Condition utils.QueryCondition `json:"condition"`
}

func (location *PotentialSomaLocation) String() string {
	jsonres, err := json.Marshal(location)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (location *PotentialSomaLocation) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), location); err != nil {
		return nil, err
	}
	return location, nil
}

func QueryPotentialSomaLocation(pa *models.TPotentialsomalocation, pd *utils.QueryCondition) ([]*PotentialSomaLocation, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	locations := make([]*models.TPotentialsomalocation, 0)
	session := utils.DB.Where("Isdeleted = ?", 0)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Length)
	}
	err := session.Find(locations, pa)

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
