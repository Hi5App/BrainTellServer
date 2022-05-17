package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
	"strconv"
)

type ArborResult struct {
	ArborId int
	Result  int
	Form    int
	Owner   string
}

func InsertArborResult(pa []*models.TArborresult) (int64, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	affected, err := utils.DB.Insert(pa)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Insert ArborResult",
			"pa":    jsonpa,
			"err":   err,
		}).Infof("Failed")
		return 0, err
	}

	log.WithFields(log.Fields{
		"event":  "Insert ArborResult",
		"pa":     jsonpa,
		"affect": affected,
	}).Infof("Success")

	return affected, nil
}

func DeleteArborResult(pa []int, user string) (int64, error) {
	//todo
	return 0, nil
}

func QueryArborResult(pa *models.TArborresult) ([]*ArborResult, error) {
	session := utils.DB.NewSession().Where("Isdeleted = ?", 0).And("ArborId = ?", pa.Arborid)
	rows := make([]*models.TArborresult, 0)
	err := session.Find(&rows)
	if err != nil {
		return nil, err
	}
	res := make([]*ArborResult, 0)
	for _, v := range rows {
		res = append(res, &ArborResult{
			ArborId: v.Arborid,
			Result:  v.Result,
			Form:    v.Form,
			Owner:   v.Owner,
		})
	}
	return res, nil
}

func QueryArborGroupByUser(isToday bool) (map[string]int64, error) {
	var sql string
	if !isToday {
		sql = "select Owner as Name, count(*) as CheckNum from t_arborresult where Isdeleted = 0 group by Owner order by CheckNum DESC"
	} else {
		sql = "select Owner as Name, count(*) as CheckNum from t_arborresult where Isdeleted = 0 and date_format(ctime,'%Y%m%d')=date_format(now(),'%Y%m%d') group by Owner order by CheckNum DESC"
	}

	resultsSlice, err := utils.DB.Query(sql)
	if err != nil {
		return nil, err
	}
	res := make(map[string]int64)
	for _, v := range resultsSlice {
		num, _ := strconv.Atoi(string(v["CheckNum"]))
		res[string(v["Name"])] = int64(num)
	}
	return res, err
}
