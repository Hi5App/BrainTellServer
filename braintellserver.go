package main

import (
	"BrainTellServer/services"
	"BrainTellServer/utils"
	"fmt"
	log "github.com/sirupsen/logrus"
	"net/http"
)

func main() {
	err := utils.Loadconfig()
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Load Config",
		}).Fatal(http.ListenAndServe("localhost:8000", nil))
	}
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Test")
	})
	//user service
	http.HandleFunc("/user/register", services.Register)
	http.HandleFunc("/user/login", services.Login)
	http.HandleFunc("/user/setuserscore", services.SetUserScore)
	http.HandleFunc("/user/updatepasswd", services.UpdatePasswd)
	http.HandleFunc("/user/forgetpasswd", services.ForgetPasswd)
	http.HandleFunc("/user/resetpasswd", services.ResetPasswd)
	http.HandleFunc("/user/registernetease", services.RegisterNetease)
	//add soma service
	http.HandleFunc("/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	http.HandleFunc("/soma/getsomalist", services.GetSomaList)
	http.HandleFunc("/soma/insertsomalist", services.InsertSomaList)
	//image service
	http.HandleFunc("/image/getimagelist", services.GetImageList)
	http.HandleFunc("/image/cropimage", services.CropImage)
	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe("localhost:8000", nil))
}
