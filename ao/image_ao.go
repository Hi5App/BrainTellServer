package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"encoding/json"
)

func GetImageList(image *do.Image) (string, error) {
	str, err := utils.GetImageFromRDB()
	if err == nil {
		return str, nil
	}
	res, err := do.QueryImage(&models.TImage{Name: image.Name}, nil)
	if err != nil {
		return "", err
	}
	bytes, err := json.Marshal(res)
	if err != nil {
		return "", err
	}
	utils.InsertImage2RDB(res)
	return string(bytes), nil
}
