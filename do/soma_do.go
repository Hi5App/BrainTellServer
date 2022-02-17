package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"fmt"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
	"strconv"
	"strings"
)

type SomaInfo struct {
	Name     string    `json:"name"`
	Image    string    `json:"image"`
	Loc      utils.XYZ `json:"loc"`
	Owner    string    `json:"owner"`
	Color    string    `json:"color"`
	Location int       `json:"location"`
	Status   int       `json:"status"`
}

func QuerySoma(pa1, pa2 *utils.XYZ, image string, pd *utils.QueryCondition) ([]*SomaInfo, error) {
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

func QuerySomaGroupByUser(isToday bool) (map[string]int64, error) {
	var sql string
	if !isToday {
		sql = "select Owner as Name, count(*) as SomaNum from t_somainfo where Isdeleted = 0 and ctime group by Owner order by SomaNum DESC"
	} else {
		sql = "select Owner as Name, count(*) as SomaNum from t_somainfo where Isdeleted = 0 and date_format(ctime,'%Y%m%d')=date_format(now(),'%Y%m%d') group by Owner order by SomaNum DESC"
	}

	resultsSlice, err := utils.DB.Query(sql)
	if err != nil {
		return nil, err
	}
	res := make(map[string]int64)
	for _, v := range resultsSlice {
		num, _ := strconv.Atoi(string(v["SomaNum"]))
		res[string(v["Name"])] = int64(num)
	}
	return res, err
}

func InsertSoma(pa []*models.TSomainfo) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)
	sql := "INSERT IGNORE INTO t_somainfo (Name,Image,X,Y,Z,Location,Owner) values "
	paras := make([]string, 0)
	for _, v := range pa {

		paras = append(paras, fmt.Sprintf("(\"%s\",\"%s\",%f,%f,%f,%d,\"%s\")",
			v.Name, v.Image, v.X, v.Y, v.Z, v.Location, v.Owner))
	}
	sql += strings.Join(paras, ",")

	affect, err := utils.DB.Exec(sql)
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

	return affect.RowsAffected()
}

func DeleteSoma(pa []string, user string) (int64, error) {

	names := make([]string, 0)
	for _, v := range pa {
		names = append(names, "\""+v+"\"")
	}

	log.Info(names)

	sql := fmt.Sprintf("UPDATE t_somainfo SET (Isdeleted,Updater) =(1,%s) WHERE Name IN (%s)", user, strings.Join(names, ","))
	sqlres, err := utils.DB.Exec(sql)
	if err != nil {
		return 0, err
	}

	return sqlres.RowsAffected()
}

func QueryLastSoma(pa *models.TSomainfo) (*SomaInfo, error) {
	var tmp models.TSomainfo
	ok, err := utils.DB.Where("Image = ?", pa.Image).Desc("ctime").Desc("Id").Get(&tmp)
	if err != nil {
		return nil, err
	}
	if !ok {
		return &SomaInfo{}, nil
	}
	return &SomaInfo{Name: tmp.Name}, nil
}
