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

func GetBoutonArbors(owner string, maxId int64) ([]*do.Arbor, error) {
	arbors, err := do.QueryBoutonArbors(owner, maxId)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "GetBoutonArbors",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return nil, err
	}
	return arbors, nil
}

func GetBoutonArborImage(arborId string) (string, error) {

	log.WithFields(log.Fields{
		"event": "GetBoutonArborImage",
		"desc":  "arbor Id" + arborId,
	}).Infof("")

	imagePath, err := do.GetBoutonArborImagePath(arborId)

	log.WithFields(log.Fields{
		"event": "GetBoutonArborImage",
		"desc":  "image path" + imagePath,
	}).Infof("")

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Get Bouton Arbor Image",
			"desc":  "Query MySQL failed",
		}).Warnf("%v\n", err)
		return "", err
	}
	return imagePath, nil
}
