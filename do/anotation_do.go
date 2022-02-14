package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
)

type TAnotation struct {
	Name   string `json:"name"`
	Neuron string `json:"neuron"`
	Owner  string `json:"owner"`
}

func GetAno(pa *models.TAnotation) ([]*TAnotation, error) {
	anos := make([]*models.TAnotation, 0)
	err := utils.DB.Where("Isdeleted = ?", 0).Find(anos, pa)
	if err != nil {
		return nil, err
	}
	res := make([]*TAnotation, 0)
	for _, ano := range anos {
		res = append(res, &TAnotation{
			Name:   ano.Name,
			Neuron: ano.Soma,
			Owner:  ano.Owner,
		})
	}
	return res, nil
}
