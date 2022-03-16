package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
)

func GetPotentialSomaLocation() ([]*do.PotentialSomaLocation, error) {
	locations, err := do.QueryPotentialSomaLocation(&models.TPotentialsomalocation{}, &utils.QueryCondition{
		Limit: utils.QueueSize, Off: 0,
	})
	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return locations, nil
}
