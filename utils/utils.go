package utils

import (
	"fmt"
	log "github.com/sirupsen/logrus"
	"io"
	"io/ioutil"
	"net/http"
	"os"
)

type QueryCondition struct {
	Limit int `json:"limit"` //数量
	Off   int `json:"off"`   //起始位置
}

type XYZ struct {
	X float64 `json:"x"`
	Y float64 `json:"y"`
	Z float64 `json:"z"`
}

type UserInfo struct {
	Id       int    `json:"id"`
	Name     string `json:"name"`
	Email    string `json:"email"`
	NickName string `json:"nickname"`
	Score    int    `json:"score"`
	AppKey   string `json:"appkey"`
	Passwd   string `json:"passwd"`
}

type Image struct {
	Name   string `json:"name"`
	Detail string `json:"detail"`
}

func DecodeFromHttp(r *http.Request, pa RequestParam) (RequestParam, error) {
	s, err := ioutil.ReadAll(r.Body)
	//todo decode
	if err != nil {
		log.WithFields(log.Fields{
			"event": "DecodeFromHttp",
			"desc":  "read failed from r.body",
		}).Warnf("%v\n", err)
		return nil, err
	}

	log.WithFields(log.Fields{
		"event": "DecodeFromHttp",
	}).Infof("%v\n", string(s))
	return pa.FromJsonString(string(s))
}

func EncodeToHttp(w http.ResponseWriter, status int, pa string) {
	//todo encdeo
	w.WriteHeader(status)
	fmt.Fprintln(w, pa)
}

func SendFile(w http.ResponseWriter, status int, pa string) {
	w.WriteHeader(status)
	f, err := os.Open(pa)
	defer f.Close()

	if err != nil {
		w.WriteHeader(502)
		return
	}
	w.WriteHeader(200)
	w.Header().Set("Content-Disposition", pa)
	io.Copy(w, f)
	os.Remove(pa)
}
