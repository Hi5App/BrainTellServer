package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/do"
	"BrainTellServer/utils"
	"encoding/json"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
	"os/exec"
	"strings"
)

func CreateFromZero(w http.ResponseWriter, r *http.Request) {
	//todo
}

func CreateFromOther(w http.ResponseWriter, r *http.Request) {
	//todo
}

type InheritOtherParam struct {
	Ano    string          `json:"ano"`
	Image  string          `json:"image"`
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
	Port string `json:"port"`
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

	port, err := utils.QueryAnoPort(p.Ano)
	if err != nil {
		utils.EncodeToHttp(w, 502, "can not allocate Port,"+err.Error())
		return
	}

	if port == "" {
		port, err = utils.AllocatePort(p.Ano)
		if err != nil {
			if port == "" {
				utils.EncodeToHttp(w, 503, "can not allocate Port,"+err.Error())
				return
			} else {
				utils.EncodeToHttp(w, 504, "can not allocate Port,"+err.Error())
				return
			}
		}

		log.Infoln(fmt.Sprintf("%s %s %s %s %s %s &", utils.CollaborateBinPath, port, utils.MainPath, p.Image, p.Neuron, p.Ano))
		cmd := exec.Command("/bin/sh", "-c", fmt.Sprintf("nohup %s %s %s %s %s %s &", utils.CollaborateBinPath, port, utils.MainPath, p.Image, p.Neuron, p.Ano))
		if err := cmd.Start(); err != nil {
			log.Error(err.Error())
		}
		if err := cmd.Wait(); err != nil {
			log.Error(err.Error())
		}
		log.Infoln("start process " + p.Ano + " success")
	}

	jsonbody, err := json.Marshal(&AllocateAnoPort{
		Ano:  p.Ano,
		Port: port,
	})

	if err != nil {
		utils.EncodeToHttp(w, 505, err.Error())
		return
	}
	utils.EncodeToHttp(w, 200, string(jsonbody))
}

type GetAnoImageParam struct {
	User UserVerifyParam `json:"user"`
}

func (image *GetAnoImageParam) String() string {
	jsonres, err := json.Marshal(image)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (image *GetAnoImageParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), image); err != nil {
		return nil, err
	}
	return image, nil
}

func GetAnoImage(w http.ResponseWriter, r *http.Request) {
	var p GetAnoImageParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	_, ok := param.(*GetAnoImageParam)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(p.User.Passwd) == 0 || (len(p.User.Name) == 0 && len(p.User.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Errorf("%s\n", p)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	images, err := ao.GetEffectSomaImage()
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
	}
	utils.EncodeToHttp(w, 200, strings.Join(images, ","))

}

type GetAnoNeuronParam struct {
	Image string          `json:"image"`
	User  UserVerifyParam `json:"user"`
}

func (pa *GetAnoNeuronParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetAnoNeuronParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func GetAnoNeuron(w http.ResponseWriter, r *http.Request) {
	var p GetAnoNeuronParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	_, ok := param.(*GetAnoNeuronParam)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(p.User.Passwd) == 0 || (len(p.User.Name) == 0 && len(p.User.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Errorf("%s\n", p)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if _, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 401, err.Error())
		return
	}

	neurons, err := ao.GetEffectSoma(p.Image)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
	}
	str, err := json.Marshal(neurons)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
	}

	utils.EncodeToHttp(w, 200, string(str))
}

type GetAnoParam struct {
	Neuron string          `json:"neuron"`
	User   UserVerifyParam `json:"user"`
}

func (pa *GetAnoParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetAnoParam) FromJsonString(jsonstr string) (utils.RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

func GetAno(w http.ResponseWriter, r *http.Request) {
	var p GetAnoParam
	param, err := utils.DecodeFromHttp(r, &p)
	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
		return
	}
	_, ok := param.(*GetAnoParam)

	if !ok {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Errorf("%v\n", err)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if len(p.User.Passwd) == 0 || (len(p.User.Name) == 0 && len(p.User.Email) == 0) {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "Bad Param",
		}).Errorf("%s\n", p)
		utils.EncodeToHttp(w, 400, "Bad Request")
		return
	}

	if user, err := ao.Login(&do.UserInfo{
		Name:   p.User.Name,
		Passwd: p.User.Passwd,
	}); err != nil {
		log.WithFields(log.Fields{
			"event": "Login",
			"desc":  "param.(*do.UserInfo) failed",
		}).Warnf("%v\n", err)
		utils.EncodeToHttp(w, 401, err.Error())
		return
	} else {
		w.Header().Set("Set-Cookie", fmt.Sprint(user.Id))
	}

	anos, err := ao.GetAno(p.Neuron)

	if err != nil {
		utils.EncodeToHttp(w, 500, err.Error())
	}
	str, err := json.Marshal(anos)
	if err != nil {
		utils.EncodeToHttp(w, 501, err.Error())
	}

	utils.EncodeToHttp(w, 200, string(str))
}
