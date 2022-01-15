package main

import (
	"BrainTellServer/services"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func main() {
	err := utils.LoadConfig()
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Load Config",
		}).Fatal(http.ListenAndServe("localhost:8000", nil))
	}
	http.HandleFunc("/dynamic/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Test")
	})
	//user service
	http.HandleFunc("/dynamic/user/register", services.Register)
	http.HandleFunc("/dynamic/user/login", services.Login)
	http.HandleFunc("/dynamic/user/setuserscore", services.SetUserScore)       //todo
	http.HandleFunc("/dynamic/user/updatepasswd", services.UpdatePasswd)       //todo
	http.HandleFunc("/dynamic/user/forgetpasswd", services.ForgetPasswd)       //todo
	http.HandleFunc("/dynamic/user/resetpasswd", services.ResetPasswd)         //todo
	http.HandleFunc("/dynamic/user/registernetease", services.RegisterNetease) //todo
	//add soma service
	http.HandleFunc("/dynamic/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	http.HandleFunc("/dynamic/soma/getsomalist", services.GetSomaList)
	http.HandleFunc("/dynamic/soma/updatesomalist", services.UpdateSomaList)
	//image service
	http.HandleFunc("/dynamic/image/getimagelist", services.GetImageList)
	http.HandleFunc("/dynamic/image/cropimage", services.CropImage)
	//
	http.HandleFunc("/dynamic/musics", func(w http.ResponseWriter, r *http.Request) {
		res, err := utils.GetMusicListFromRDB()
		if err != nil {
			w.WriteHeader(500)
			return
		}
		utils.EncodeToHttp(w, 200, res)
	})
	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8000", nil))
}
