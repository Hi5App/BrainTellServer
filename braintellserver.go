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
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Test")
	})
	//user service
	http.HandleFunc("/user/register", services.Register)
	http.HandleFunc("/user/login", services.Login)
	http.HandleFunc("/user/setuserscore", services.SetUserScore)       //todo
	http.HandleFunc("/user/updatepasswd", services.UpdatePasswd)       //todo
	http.HandleFunc("/user/forgetpasswd", services.ForgetPasswd)       //todo
	http.HandleFunc("/user/resetpasswd", services.ResetPasswd)         //todo
	http.HandleFunc("/user/registernetease", services.RegisterNetease) //todo
	//add soma service
	http.HandleFunc("/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	http.HandleFunc("/soma/getsomalist", services.GetSomaList)
	http.HandleFunc("/soma/insertsomalist", services.InsertSomaList)
	//image service
	http.HandleFunc("/image/getimagelist", services.GetImageList)
	http.HandleFunc("/image/cropimage", services.CropImage)
	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8000", nil))
}
