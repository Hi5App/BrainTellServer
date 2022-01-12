package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"fmt"
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

func InsertSomaList(pa *utils.InsertSomaListParam) error {
	lastsoma, err := do.QueryLastSoma(&models.TSomainfo{
		Image: pa.Image,
	})
	if err != nil {
		return err
	}
	idx := 0
	if len(lastsoma.Name) != 0 {
		idx, err = strconv.Atoi(strings.Split(lastsoma.Name, "_")[1])
		if err != nil {
			idx = 0
		}
	}
	somalist := pa.Somalist
	for i := range somalist {
		somalist[i].Owner = pa.Owner
		somalist[i].Image = pa.Image
		somalist[i].Name = pa.Image + "_" + fmt.Sprintf("%05d", idx+i+1)
		somalist[i].Location = pa.LocationId
	}
	_, err = do.InsertSoma(somalist)
	if err != nil {
		return err
	}

	_, err = do.UpdatePotentialSomaLocation(&models.TPotentialsomalocation{
		Id:    pa.LocationId,
		Owner: pa.Owner,
	})
	if err != nil {
		return err
	}

	return nil
}
