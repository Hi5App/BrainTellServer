package ao

import (
	"BrainTellServer/do"
	log "github.com/sirupsen/logrus"
)

func GetArbors(owner string) ([]*do.Arbor, error) {
	arbors, err := do.QueryArbors(owner)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetArbors",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return arbors, nil
}
