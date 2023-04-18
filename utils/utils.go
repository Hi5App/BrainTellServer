package utils

import (
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
	"io"
	"io/ioutil"
	"net/http"
	"os"
	"reflect"
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

type GameUserInfo struct {
	Id     int    `json:"id"`
	Name   string `json:"name"`
	Email  string `json:"email"`
	Score  int    `json:"score"`
	Passwd string `json:"passwd"`
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

func In(haystack interface{}, needle interface{}) (bool, error) {
	//获得haystack的反射值对象
	sVal := reflect.ValueOf(haystack)
	//获得haystack的种类名称
	kind := sVal.Kind()
	if kind == reflect.Slice || kind == reflect.Array {
		for i := 0; i < sVal.Len(); i++ {
			if sVal.Index(i).Interface() == needle {
				return true, nil
			}
		}
		return false, nil
	}
	return false, errors.New("")
}

// 排序的同时返回其排序前索引
func Sort(array []int) []int {
	var n int = len(array)
	var index []int
	for i := 0; i < n; i++ {
		index = append(index, i)
	}
	// fmt.Println(index)
	// fmt.Println("数组array的长度为：", n)
	if n < 2 {
		return nil
	}
	for i := 1; i < n; i++ {
		// fmt.Printf("检查第%d个元素%f\t", i, array[i])
		var temp int = array[i]
		var tempIndex = index[i]
		var k int = i - 1
		for k >= 0 && array[k] > temp {
			k--
		}
		for j := i; j > k+1; j-- {
			array[j] = array[j-1]
			index[j] = index[j-1]
		}
		// fmt.Printf("其位置为%d\n", k+1)
		array[k+1] = temp
		index[k+1] = tempIndex
	}
	return index
}
