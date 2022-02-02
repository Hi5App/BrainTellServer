package utils

import (
	"BrainTellServer/models"
	"encoding/json"
	log "github.com/sirupsen/logrus"
)

type RequestParam interface {
	String() string
	FromJsonString(jsonstr string) (RequestParam, error)
}

type Image struct {
	Name      string         `json:"name"`
	Detail    string         `json:"detail"`
	User      UserInfo       `json:"user"`
	Condition QueryCondition `json:"condition"`
}

func (image *Image) String() string {
	jsonres, err := json.Marshal(image)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (image *Image) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), image); err != nil {
		return nil, err
	}
	return image, nil
}

type PotentialSomaLocation struct {
	Id        int64          `json:"id"`
	Image     string         `json:"image"`
	Loc       XYZ            `json:"loc"`
	Owner     string         `json:"owner"`
	User      UserInfo       `json:"user"`
	Condition QueryCondition `json:"condition"`
}

func (location *PotentialSomaLocation) String() string {
	jsonres, err := json.Marshal(location)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (location *PotentialSomaLocation) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), location); err != nil {
		return nil, err
	}
	return location, nil
}

type SomaInfo struct {
	Name     string `json:"name"`
	Image    string `json:"image"`
	Loc      XYZ    `json:"loc"`
	Owner    string `json:"owner"`
	Color    string `json:"color"`
	Location int    `json:"location"`
	Status   int    `json:"status"`
}

func (soma *SomaInfo) String() string {
	jsonres, err := json.Marshal(soma)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (soma *SomaInfo) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), soma); err != nil {
		return nil, err
	}
	return soma, nil
}

type QuerySomaListParam struct {
	Pa1       XYZ            `json:"pa1"`
	Pa2       XYZ            `json:"pa2"`
	Image     string         `json:"image"`
	User      UserInfo       `json:"user"`
	Condition QueryCondition `json:"condition"`
}

func (param *QuerySomaListParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *QuerySomaListParam) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

type UpdateSomaListParam struct {
	LocationId     int                 `json:"locationId"`
	InsertSomalist []*models.TSomainfo `json:"insertsomalist"`
	DeleteSomalist []string            `json:"deletesomalist"`
	Owner          string              `json:"owner"`
	Image          string              `json:"image"`
	User           UserInfo            `json:"user"`
}

func (param *UpdateSomaListParam) String() string {
	jsonres, err := json.Marshal(param)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (param *UpdateSomaListParam) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), param); err != nil {
		return nil, err
	}
	return param, nil
}

type UserInfo struct {
	Name     string `json:"name"`
	Email    string `json:"email"`
	NickName string `json:"nickname"`
	Score    int    `json:"score"`
	AppKey   string `json:"appkey"`
	Passwd   string `json:"passwd"`
	NPasswd  string `json:"npasswd"`
}

func (user *UserInfo) String() string {
	jsonres, err := json.Marshal(user)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (user *UserInfo) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), user); err != nil {
		log.WithFields(log.Fields{
			"event": "Query userinfo",
			"pa":    jsonstr,
		}).Warnf("%s\n%v\n", string([]byte(jsonstr)), err)
		return nil, err
	}
	return user, nil
}

type CropBB struct {
	Loc    XYZ      `json:"loc"`
	Image  string   `json:"image"`
	RES    string   `json:"res"`
	RESIdx int      `json:"residx"`
	User   UserInfo `json:"user"`
	Len    int      `json:"len"`
}

func (pa *CropBB) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *CropBB) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		return nil, err
	}
	return pa, nil
}

type GetLatestApkParam struct {
	Version string `json:"version"`
}

func (pa *GetLatestApkParam) String() string {
	jsonres, err := json.Marshal(pa)
	if err != nil {
		return ""
	}
	return string(jsonres)
}

func (pa *GetLatestApkParam) FromJsonString(jsonstr string) (RequestParam, error) {
	if err := json.Unmarshal([]byte(jsonstr), pa); err != nil {
		log.WithFields(log.Fields{
			"event": "Query userinfo",
			"pa":    jsonstr,
		}).Warnf("%s\n%v\n", string([]byte(jsonstr)), err)
		return nil, err
	}
	return pa, nil
}