package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	log "github.com/sirupsen/logrus"
)

type UpdateArboResultAo struct {
	Insertlist []*models.TArborresult `json:"insertlist"`
}

func UpdateArborResult(pa *UpdateArboResultAo) error {

	_, err := do.InsertArborResult(pa.Insertlist)
	if err != nil {
		log.WithFields(log.Fields{
			"event":  "UpdateSomaList",
			"action": "InsertSomas",
		}).Warn(err)
		return err
	}
	return nil
}

func QueryArborResult(arborid int) ([]*do.ArborResult, error) {
	return do.QueryArborResult(&models.TArborresult{
		Arborid: arborid,
	})
}
