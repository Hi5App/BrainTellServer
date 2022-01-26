package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
	"strconv"
	"strings"
)

func GetSomaList(pa1, pa2 *utils.XYZ, image string) ([]*utils.SomaInfo, error) {
	res, err := do.QuerySoma(pa1, pa2, image, nil)
	if err != nil {
		return nil, err
	}
	return res, nil
}

func UpdateSomaList(pa *utils.UpdateSomaListParam) error {
	//set potential

	//先判断互斥锁过期时间
	//1.不存在或者小于30s 返回错误
	//2.存在且时间大于30s 获取互斥锁持有人
	//3.当前持有人不是发起人，返回错误
	//4.是持有人，修改过期时间10分钟，返回正常
	//5.设置
	if _, err := utils.GetLocationTTL("PotentialSomaLocation" + fmt.Sprint(pa.LocationId)); err != nil {
		return err
	}
	user, err := utils.GetLocationValue("PotentialSomaLocation" + fmt.Sprint(pa.LocationId))
	if err != nil {
		return err
	}
	if user != pa.User.Name {
		return errors.New(fmt.Sprintf("Not right user,%s %s\n", user, pa.User.Name))
	}

	if utils.SetKeyTTL("PotentialSomaLocation"+fmt.Sprint(pa.LocationId), 10*60) != nil {
		return err
	}
	_, err = do.UpdatePotentialSomaLocation(&models.TPotentialsomalocation{
		Id:    pa.LocationId,
		Owner: pa.Owner,
	})

	if err != nil {
		return err
	}

	if len(pa.DeleteSomalist) == 0 && len(pa.InsertSomalist) == 0 {
		return nil
	}
	//delete
	if len(pa.DeleteSomalist) != 0 {
		_, err := do.DeleteSoma(pa.DeleteSomalist)
		if err != nil {
			log.WithFields(log.Fields{
				"event":  "UpdateSomaList",
				"action": "delete",
			}).Warn(err)
			return err
		}
	}

	if len(pa.InsertSomalist) == 0 {
		return nil
	}
	//insert
	lastsoma, err := do.QueryLastSoma(&models.TSomainfo{
		Image: pa.Image,
	})
	if err != nil {
		if err != nil {
			log.WithFields(log.Fields{
				"event":  "UpdateSomaList",
				"action": "QueryLastSoma",
			}).Warn(err)
			return err
		}
		return err
	}

	idx := 0
	if len(lastsoma.Name) != 0 {
		idx, err = strconv.Atoi(strings.Split(lastsoma.Name, "_")[1])
		if err != nil {
			idx = 0
		}
	}

	somalist := pa.InsertSomalist
	for i := range somalist {
		somalist[i].Owner = pa.Owner
		somalist[i].Image = pa.Image
		somalist[i].Name = pa.Image + "_" + fmt.Sprintf("%05d", idx+i+1)
		somalist[i].Location = pa.LocationId
	}
	_, err = do.InsertSoma(somalist)
	if err != nil {
		log.WithFields(log.Fields{
			"event":  "UpdateSomaList",
			"action": "InsertSomas",
		}).Warn(err)
		return err
	}

	return nil
}
