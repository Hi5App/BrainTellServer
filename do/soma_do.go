package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type SomaInfo struct {
	Name  string    `json:"name"`
	Image string    `json:"image"`
	Loc   utils.XYZ `json:"loc"`
	Owner string    `json:"owner"`
	Color string    `json:"color"`
}

func (soma *SomaInfo) String() string {
	jsonres, err := json.Marshal(soma)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (soma *SomaInfo) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), soma); err != nil {
		return nil, err
	}
	return soma, nil
}

type QuerySomaListParam struct {
	Pa1       utils.XYZ            `json:"pa1"`
	Pa2       utils.XYZ            `json:"pa2"`
	Image     string               `json:"image"`
	User      UserInfo             `json:"user"`
	Condition utils.QueryCondition `json:"condition"`
}

func (param *QuerySomaListParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *QuerySomaListParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

type InsertSomaListParam struct {
	Somalist []*SomaInfo `json:"somalist"`
	Owner    string      `json:"owner"`
	User     UserInfo    `json:"user"`
}

func (param *InsertSomaListParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *InsertSomaListParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

func QuerySoma(pa1, pa2 *utils.XYZ, image string, pd *utils.QueryCondition) ([]*SomaInfo, error) {
	jsonpa1, _ := jsoniter.MarshalToString(pa1)
	jsonpa2, _ := jsoniter.MarshalToString(pa2)
	somas := make([]models.TSomainfo, 0)
	session := utils.DB.Where("Isdeleted = ?", 0).And("Image = ?", image)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Length)
	}
	err := session.Where("X >= ? and Y >= ? and Z >= ?", pa1.X, pa1.Y, pa1.Z).
		And("X <= ? and Y <= ? and Z <= ?", pa2.X, pa2.Y, pa2.Z).Find(somas)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query SomaInfo",
			"pa1":   jsonpa1,
			"pa2":   jsonpa2,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*SomaInfo, 0)
	for _, soma := range somas {
		res = append(res, &SomaInfo{
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
