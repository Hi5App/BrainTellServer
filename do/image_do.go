package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

func QueryImage(pa *models.TImage, pd *utils.QueryCondition) ([]*utils.Image, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	images := make([]*models.TImage, 0)
	session := utils.DB.Where("Isdeleted = ?", 0)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Off)
	}
	err := session.Find(&images, pa)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query image",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*utils.Image, 0)
	for _, image := range images {
		res = append(res, &utils.Image{
			Name:   image.Name,
			Detail: image.Detail,
		})
	}
	jsonres, _ := jsoniter.MarshalToString(res)
	log.WithFields(log.Fields{
		"event": "Query image",
		"pa":    jsonpa,
		"RES":   jsonres,
	}).Infof("Success")
	return res, nil
	return nil, nil
}
