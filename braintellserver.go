package main

import (
	"BrainTellServer/services"
	"BrainTellServer/utils"
	"net/http"

	log "github.com/sirupsen/logrus"
)

func main() {
	err := utils.LoadConfig()
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Load Config",
		}).Fatal()
	}
	services.SendPerformance()

	//http.HandleFunc("/dynamic/", func(w http.ResponseWriter, r *http.Request) {
	//	fmt.Fprintln(w, "dynamic")
	//})
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
	//check arbor service
	http.HandleFunc("/dynamic/arbor/getarbor", services.GetArbor)
	http.HandleFunc("/dynamic/arbor/queryarborresult", services.QueryArborResult)
	http.HandleFunc("/dynamic/arbor/updatearborresult", services.UpdateArborResult)
	//增加一些
	http.HandleFunc("/dynamic/arbordetail/insert", services.InsertArborDetail)
	//删一些
	http.HandleFunc("/dynamic/arbordetail/delete", services.DeleteArbordetail)
	//查一个
	http.HandleFunc("/dynamic/arbordetail/query", services.QueryArborDetail)

	//collaborate service
	http.HandleFunc("/dynamic/collaborate/getanoimage", services.GetAnoImage)
	http.HandleFunc("/dynamic/collaborate/getanoneuron", services.GetAnoNeuron)
	http.HandleFunc("/dynamic/collaborate/getano", services.GetAno)

	//给某个ano文件分配端口
	http.HandleFunc("/dynamic/collaborate/inheritother", services.InheritOther)
	http.HandleFunc("/dynamic/collaborate/createnewanofromzero", services.CreateFromZero)
	http.HandleFunc("/dynamic/collaborate/createnewanofromother", services.CreateFromOther)

	//image service
	http.HandleFunc("/dynamic/image/getimagelist", services.GetImageList)
	//获取裁剪后的图像文件.v3dpbd
	http.HandleFunc("/dynamic/image/cropimage", services.CropImage)
	//获取裁剪后的eswc文件
	http.HandleFunc("/dynamic/swc/cropswc", services.CropSWC)
	http.HandleFunc("/dynamic/apo/cropapo", services.CropApo)
	//resource service
	http.HandleFunc("/dynamic/musics", services.GetMusicList)
	http.HandleFunc("/dynamic/updateapk", services.GetLatestApk)

	// game server
	//http.HandleFunc("/game/dynamic/user/register", services.GameRegister)
	http.HandleFunc("/dynamic/game/user/login", services.GameLogin)
	//http.HandleFunc("/game/dynamic/user/uploadrecord", services.GameUpdataRecord)
	http.HandleFunc("/dynamic/game/swc/bppoint/insert", services.InsertBranchingPoints)

	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8000", nil))
}
