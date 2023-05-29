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
	//根据ArborId和Owner修改表中已存在的记录的result,并向表中插入不存在的记录
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

// bouton相关操作
func UpdateBoutonArborResult(pa *UpdateArboResultAo) error {
	insertlist := make([]*models.TArborresultBouton, 0)
	for _, v := range pa.Insertlist {
		insertlist = append(insertlist, &models.TArborresultBouton{
			Arborid: v.ArborId,
			Result:  v.Result,
			Form:    v.Form,
			Owner:   v.Owner,
		})
	}
	//根据ArborId和Owner修改表中已存在的记录的result,并向表中插入不存在的记录
	_, err := do.InsertBoutonArborResult(insertlist)
	if err != nil {
		log.WithFields(log.Fields{
			"event":  "UpdateBoutonArborResult",
			"action": "InsertBoutonArbor",
		}).Warn(err)
		return err
	}
	return nil
}

func QueryBoutonArborResult(arborid int) ([]*do.ArborResult, error) {
	return do.QueryBoutonArborResult(&models.TArborresultBouton{
		Arborid: arborid,
	})
}
