package ao

import (
	"BrainTellServer/do"
	log "github.com/sirupsen/logrus"
)

func GetArbors(owner string, maxId int64) ([]*do.Arbor, error) {
	arbors, err := do.QueryArbors(owner, maxId)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetArbors",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return arbors, nil
}
