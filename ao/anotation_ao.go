package ao

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
)

func GetAno(neuron string) ([]*do.TAnotation, error) {
	anos, err := do.GetAno(&models.TAnotation{
		Soma: neuron,
	})
	if err != nil {
		return nil, err
	}
	return anos, err
}
