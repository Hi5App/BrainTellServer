package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
)

func InsertBP(pa []*models.TGameRecord) error {

	_, err := do.InsertBP(pa)
	if err != nil {
		return err
	}
	return nil

}
