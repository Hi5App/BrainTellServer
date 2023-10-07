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

// bouton相关
func InsertBoutonArborDetail(pa []*models.TArbordetailBouton) (int, error) {
	affected, err := do.InsetBoutonArborDetail(pa)
	if err != nil {
		return 0, err
	}
	return affected, nil
}

func DeleteBoutonArbordetail(pa []*models.TArbordetailBouton, username string) (int, error) {
	affected, err := do.DeleteBoutonArbordetail(pa, username)
	if err != nil {
		return 0, err
	}
	return affected, nil
}

func QueryBoutonArborDetail(pa *models.TArbordetailBouton) ([]*do.ArborDetail, error) {
	res, err := do.QueryBoutonArborDetail(pa)
	if err != nil {
		return nil, err
	}
	return res, nil
}
