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

	//services.SendPerformance()

	http.HandleFunc("/dynamic/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Test")
	})
	//user service
	http.HandleFunc("/dynamic/user/register", services.Register)
	http.HandleFunc("/dynamic/user/login", services.Login)
	http.HandleFunc("/dynamic/user/setuserscore", services.SetUserScore)       //todo
	http.HandleFunc("/dynamic/user/updatepasswd", services.UpdatePasswd)       //todo
	http.HandleFunc("/dynamic/user/resetpasswd", services.ResetPasswd)         //todo
	http.HandleFunc("/dynamic/user/registernetease", services.RegisterNetease) //todo
	http.HandleFunc("/dynamic/user/getuserperformance", services.GetUserPerformance)
	//add soma service
	http.HandleFunc("/dynamic/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	http.HandleFunc("/dynamic/soma/getsomalist", services.GetSomaList)
	http.HandleFunc("/dynamic/soma/updatesomalist", services.UpdateSomaList)
	//collaborate service
	http.HandleFunc("/dynamic/collaborate/getanoimage", services.GetAnoImage)
	http.HandleFunc("/dynamic/collaborate/getanoneuron", services.GetAnoNeuron)
	http.HandleFunc("/dynamic/collaborate/getano", services.GetAno)

	http.HandleFunc("/dynamic/collaborate/createnewanofromzero", services.CreateFromZero)
	http.HandleFunc("/dynamic/collaborate/createnewanofromother", services.CreateFromOther)
	http.HandleFunc("/dynamic/collaborate/inheritother", services.InheritOther)
	//image service
	http.HandleFunc("/dynamic/image/getimagelist", services.GetImageList)
	http.HandleFunc("/dynamic/image/cropimage", services.CropImage)
	//resource service
	http.HandleFunc("/dynamic/musics", services.GetMusicList)
	http.HandleFunc("/dynamic/updateapk", services.GetLatestApk)

	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8000", nil))
}
