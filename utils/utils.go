package utils

import (
	log "github.com/sirupsen/logrus"
	"io/ioutil"
	"net/http"
)

type QueryCondition struct {
	Limit  int
	Length int
}

type RequestParam interface {
	String() string
	FromJsonString(jsonstr string) (RequestParam, error)
}

type XYZ struct {
	X int `json:"x"`
	Y int `json:"y"`
	Z int `json:"z"`
}

func DecodeFromHttp(r *http.Request, pa RequestParam) (RequestParam, error) {
	s, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "DecodeFromHttp",
			"desc":  "read failed from r.body",
		}).Warnf("%v\n", err)
		return nil, err
	}

	log.WithFields(log.Fields{
		"event": "Register",
	}).Infof("%v\n", string(s))

	return pa.FromJsonString(string(s))

}
