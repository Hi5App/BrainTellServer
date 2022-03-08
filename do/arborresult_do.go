package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"fmt"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
	"strings"
)

type ArborResult struct {
	X         float64
	Y         float64
	Z         float64
	Type      int
	Owner     string
	ArborName string
}

func InsertArborResult(pa []*models.TArborresult) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)
	sql := "INSERT IGNORE INTO t_arborresult (X,Y,Z,Type,Owner,ArborName) values "
	paras := make([]string, 0)
	for _, v := range pa {
		paras = append(paras, fmt.Sprintf("(%f,%f,%f,%d,\"%s\",\"%s\")",
			v.X, v.Y, v.Z, v.Type, v.Owner, v.Arborname))
	}
	sql += strings.Join(paras, ",")

	affect, err := utils.DB.Exec(sql)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Insert ArborResult",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Insert ArborResult",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")

	return affect.RowsAffected()
}

func DeleteArborResult(pa []int, user string) (int64, error) {
	session := utils.DB.NewSession().Where("Isdeleted = ?", 0).In("Id", pa)
	affect, err := session.Update(&models.TArborresult{
		Updateowner: user,
		Isdeleted:   1,
	})
	if err != nil {
		log.WithFields(log.Fields{
			"event": "delete ArborResult",
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "delete ArborResult",
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}

func QueryArborResult(pa1, pa2 *utils.XYZ, image string, pd *utils.QueryCondition) ([]*SomaInfo, error) {
	return nil, nil
}
