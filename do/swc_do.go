package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type SwcDetail struct {
	Id        int    `json:"id"`
	Username  string `json:"username"`
	Swcid     string `json:"swcid"`
	Points    int    `json:"points"`
	Missedbp  string `json:"missedbp"`
	Wrongbp   string `json:"wrongbp"`
	Correctbp string `json:"correctbp"`
}

func InsertBP(pa []*models.TGameRecord) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)
	session := utils.DB.NewSession()
	defer session.Close()
	affect, err := session.Insert(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Insert BP",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Insert BP",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}
