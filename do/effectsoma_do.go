package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"fmt"
	"strconv"
)

type TEffectSoma struct {
	Name    string  `json:"name"`
	X       float64 `json:"x"`
	Y       float64 `json:"y"`
	Z       float64 `json:"z"`
	Imageid string  `json:"imageid"`
	From    int     `json:"from"`
}

func QueryEffectSomaImage() ([]string, error) {
	var images []string
	// Cols改为了Distinct
	err := utils.DB.Table("t_effect_soma").Where("Isdeleted = ?", 0).Distinct("Image").Find(&images)
	if err != nil {
		return nil, err
	}
	var numbers []int
	for _, value := range images {
		var number int
		number, _ = strconv.Atoi(value)
		numbers = append(numbers, number)
	}
	fmt.Println(numbers)

	var index []int = utils.Sort(numbers)
	var sortedImages []string
	for _, value := range index {
		sortedImages = append(sortedImages, images[value])
	}

	return sortedImages, nil

}

func QueryEffectSoma(pa *models.TEffectSoma) ([]*TEffectSoma, error) {
	somas := make([]*models.TEffectSoma, 0)
	err := utils.DB.Where("Isdeleted = ?", 0).Find(&somas, pa)
	if err != nil {
		return nil, err
	}
	res := make([]*TEffectSoma, 0)
	for _, soma := range somas {
		res = append(res, &TEffectSoma{
			Name:    soma.Name,
			X:       soma.X,
			Y:       soma.Y,
			Z:       soma.Z,
			Imageid: soma.Image,
			From:    soma.From,
		})
	}
	return res, nil
}
