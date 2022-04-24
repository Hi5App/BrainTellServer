package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/utils"
	log "github.com/sirupsen/logrus"
)

// GetSomaCnt 获取soma变动信息
func GetSomaCnt() (map[string]int64, map[string]int64, error) {
	performance, err := utils.QueryPerformance2RDB("totalsoma")
	if err != nil {
		log.Warningln(err)
		performance, err = do.QuerySomaGroupByUser(false)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("totalsoma", performance)
	}

	dailyperformance, err := utils.QueryPerformance2RDB("dailysoma")
	if err != nil {
		log.Warningln(err)
		dailyperformance, err = do.QuerySomaGroupByUser(true)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("dailysoma", dailyperformance)
	}
	return performance, dailyperformance, nil
}

func GetCheckCnt() (map[string]int64, map[string]int64, error) {
	performance, err := utils.QueryPerformance2RDB("totalcheck")
	if err != nil {
		log.Warningln(err)
		performance, err = do.QueryArborGroupByUser(false)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("totalcheck", performance)
	}

	dailyperformance, err := utils.QueryPerformance2RDB("dailycheck")
	if err != nil {
		log.Warningln(err)
		dailyperformance, err = do.QueryArborGroupByUser(true)
		if err != nil {
			log.Errorln(err)
			return nil, nil, err
		}
		utils.InsertPerformance2RDB("dailycheck", dailyperformance)
	}
	return performance, dailyperformance, nil
}
