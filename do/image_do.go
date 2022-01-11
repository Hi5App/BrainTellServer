package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
	jsoniter "github.com/json-iterator/go"
	log "github.com/sirupsen/logrus"
)

type Image struct {
	Name      string               `json:"name"`
	Detail    string               `json:"detail"`
	User      UserInfo             `json:"user"`
	Condition utils.QueryCondition `json:"condition"`
}

func (image *Image) String() string {
	jsonres, err := json.Marshal(image)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (image *Image) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), image); err != nil {
		return nil, err
	}
	return image, nil
}

func QueryImage(pa *models.TImage, pd *utils.QueryCondition) ([]*Image, error) {
	jsonpa, _ := jsoniter.MarshalToString(pa)

	images := make([]*models.TImage, 0)
	session := utils.DB.Where("Isdeleted = ?", 0)
	if pd != nil {
		session = session.Limit(pd.Limit, pd.Length)
	}
	err := session.Find(images, pa)

	if err != nil {
		log.WithFields(log.Fields{
			"event": "Query image",
			"pa":    jsonpa,
		}).Warnf("%v\n", err)
		return nil, err
	}

	res := make([]*Image, 0)
	for _, image := range images {
		res = append(res, &Image{
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
}
