package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	log "github.com/sirupsen/logrus"
)

type UpdateArboResultAo struct {
	Insertlist []*do.ArborResult `json:"insertlist"`
}

func UpdateArborResult(pa *UpdateArboResultAo) error {
	insertlist := make([]*models.TArborresult, 0)
	for _, v := range pa.Insertlist {
		insertlist = append(insertlist, &models.TArborresult{
			Arborid: v.ArborId,
			Result:  v.Result,
			Form:    v.Form,
			Owner:   v.Owner,
		})
	}
	_, err := do.InsertArborResult(insertlist)
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
