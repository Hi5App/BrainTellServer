package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
)

// GetSomaCnt 获取soma变动信息
func GetSomaCnt() (map[string]int64, map[string]int64, error) {
	performance, err := utils.QueryPerformance2RDB("performance")
	if err != nil {
		log.Warningln(err)
		performance, err = do.QuerySomaGroupByUser(false)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("performance", performance)
	}

	dailyperformance, err := utils.QueryPerformance2RDB("dailyperformance")
	if err != nil {
		log.Warningln(err)
		dailyperformance, err = do.QuerySomaGroupByUser(true)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("dailyperformance", dailyperformance)
	}
	return performance, dailyperformance, nil
}
