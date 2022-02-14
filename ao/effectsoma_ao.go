package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
)

func GetEffectSomaImage() ([]string, error) {
	images, err := do.QueryEffectSomaImage()
	if err != nil {
		return nil, err
	}
	return images, nil
}

func GetEffectSoma(image string) ([]*do.TEffectSoma, error) {
	somas, err := do.QueryEffectSoma(&models.TEffectSoma{
		Imageid: image,
	})
	if err != nil {
		return nil, err
	}
	return somas, err
}
