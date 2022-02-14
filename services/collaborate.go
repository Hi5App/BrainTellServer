package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func CreateFromZero(w http.ResponseWriter, r *http.Request) {
	//todo
}

func CreateFromOther(w http.ResponseWriter, r *http.Request) {
	//todo
}

type InheritOtherParam struct {
	Ano    string          `json:"ano"`
	Neuron string          `json:"neuron"`
	User   UserVerifyParam `json:"user"`
}

func (image *InheritOtherParam) String() string {
	jsonres, err := json.Marshal(image)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (image *InheritOtherParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), image); err != nil {
		return nil, err
	}
	return image, nil
}

type AllocateAnoPort struct {
	Ano  string `json:"ano"`
	Port int    `json:"port"`
}

func InheritOther(w http.ResponseWriter, r *http.Request) {
	var p InheritOtherParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	_, ok := param.(*InheritOtherParam)
	if !ok {
		log.WithFields(log.Fields{
			"event": "GetPotentialSomaLocations",
			"desc":  "param.(*do.PotentialSomaLocation) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	if len(p.Ano) == 0 || len(p.Neuron) == 0 {
		utils.EncodeToHttp(w, 501, "Bad Request")
		return
	}
	port, err := utils.AllocatePort(p.Ano)
	if err != nil {
		utils.EncodeToHttp(w, 502, "can not allocate Port,"+err.Error())
		return
	}
	if port == 0 {
		utils.EncodeToHttp(w, 503, "can not allocate Port")
		return
	}
	jsonbody, err := json.Marshal(&AllocateAnoPort{
		Ano:  p.Ano,
		Port: port,
	})
	if err != nil {
		utils.EncodeToHttp(w, 504, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, string(jsonbody))
}
