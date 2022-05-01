package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
)

func QueryArborDetail(pa *models.TArbordetail) ([]*do.ArborDetail, error) {
	res, err := do.QueryArborDetail(pa)
	if err != nil {
		return nil, err
	}
	return res, nil
}

func InsertArborDetail(pa []*models.TArbordetail) (int, error) {
	affected, err := do.InsetArborDetail(pa)
	if err != nil {
		return 0, err
	}
	return affected, nil
}

func DeleteArbordetail(pa []*models.TArbordetail) (int, error) {
	affected, err := do.DeleteArbordetail(pa)
	if err != nil {
		return 0, err
	}
	return affected, nil
}
