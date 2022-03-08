package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
)

func GetArbors() ([]*do.Arbor, error) {
	arbors, err := do.QueryArbors(&models.TArbor{}, &utils.QueryCondition{
		Limit: QueueSize, Off: 0,
	})
	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetArbors",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return arbors, nil
}
