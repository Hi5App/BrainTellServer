package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

func QuerySoma(pa1, pa2 *utils.XYZ, image string, pd *utils.QueryCondition) ([]*utils.SomaInfo, error) {
	jsonpa1, _ := jsoniter.MarshalToString(pa1)
	jsonpa2, _ := jsoniter.MarshalToString(pa2)
	somas := make([]models.TSomainfo, 0)
	session := utils.DB.Where("Isdeleted = ?", 0).And("Image = ?", image)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Off)
	}
	err := session.Where("X >= ? and Y >= ? and Z >= ?", pa1.X, pa1.Y, pa1.Z).
		And("X <= ? and Y <= ? and Z <= ?", pa2.X, pa2.Y, pa2.Z).Find(&somas)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query SomaInfo",
			"pa1":   jsonpa1,
			"pa2":   jsonpa2,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*utils.SomaInfo, 0)
	for _, soma := range somas {
		res = append(res, &utils.SomaInfo{
			Name:  soma.Name,
			Image: soma.Image,
			Loc: utils.XYZ{
				X: soma.X,
				Y: soma.Y,
				Z: soma.Z,
			},
			Owner: soma.Owner,
			Color: soma.Color,
		})
	}

	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query SomaInfo",
		"pa1":   jsonpa1,
		"pa2":   jsonpa2,
		"RES":   jsonres,
	}).Infof("Success")

	return res, nil
}

func InsertSoma(pa []*models.TSomainfo) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)
	affect, err := utils.DB.Insert(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Insert Soma",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Insert Soma",
		"pa":     jsonpa,
		"affect": affect,
	}).Infof("Success")
	return affect, nil
}

func QueryLastSoma(pa *models.TSomainfo) (*utils.SomaInfo, error) {
	var res models.TSomainfo
	_, err := utils.DB.Where("Image = ?", pa.Image).Desc("ctime").Get(&res)
	if err != nil {
		return nil, err
	}
	return &utils.SomaInfo{
		Name: res.Name,
	}, nil
}
