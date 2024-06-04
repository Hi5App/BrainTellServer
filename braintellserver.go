package main

import (
	"BrainTellServer/services"
	"BrainTellServer/utils"
	"net/http"

	log "github.com/sirupsen/logrus"
)

func main() {
	//var configName = "config"
	var configName = "config_release"
	err := utils.LoadConfig(configName)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Load Config",
		}).Fatal()
	}

	services.SendPerformance()

	BrainTellServerApiService := services.NewBrainTellServerApiService()
	BrainTellServerApiController := services.NewBrainTellServerApiController(BrainTellServerApiService)
	router := services.NewRouter(BrainTellServerApiController)

	// GetRatingImageFileGet - 获取打分的图片文件
	services.HandleRatingStaticImageFile(router)
	services.InitializeScheduleExpiredImageList()

	//http.HandleFunc("/release/", func(w http.ResponseWriter, r *http.Request) {
	//	fmt.Fprintln(w, "release")
	//})
	//user service
	router.HandleFunc("/release/user/register", services.Register)
	router.HandleFunc("/release/user/login", services.Login)
	router.HandleFunc("/release/user/setuserscore", services.SetUserScore)       //todo
	router.HandleFunc("/release/user/updatepasswd", services.UpdatePasswd)       //todo
	router.HandleFunc("/release/user/resetpasswd", services.ResetPasswd)         //todo
	router.HandleFunc("/release/user/registernetease", services.RegisterNetease) //todo
	router.HandleFunc("/release/user/getuserperformance", services.GetUserPerformance)
	//add soma service
	router.HandleFunc("/release/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	router.HandleFunc("/release/soma/getsomalist", services.GetSomaList)
	router.HandleFunc("/release/soma/updatesomalist", services.UpdateSomaList)
	//check arbor service
	router.HandleFunc("/release/arbor/getarbor", services.GetArbor)
	router.HandleFunc("/release/arbor/queryarborresult", services.QueryArborResult)
	router.HandleFunc("/release/arbor/updatearborresult", services.UpdateArborResult)
	//增加一些
	router.HandleFunc("/release/arbordetail/insert", services.InsertArborDetail)
	//删一些
	router.HandleFunc("/release/arbordetail/delete", services.DeleteArbordetail)
	//查一个
	router.HandleFunc("/release/arbordetail/query", services.QueryArborDetail)

	//// bouton check service
	router.HandleFunc("/release/arbor/getboutonarbor", services.GetBoutonArbor)
	router.HandleFunc("/release/arbor/getboutonarborimage", services.GetBoutonArborImage) // 临时的bouton获取服务，直接获取bouton的image文件
	router.HandleFunc("/release/arbor/updateboutonarborresult", services.UpdateBoutonArborResult)
	router.HandleFunc("/release/arbor/queryboutonarborresult", services.QueryBoutonArborResult)

	//// bouton detail的相关操作
	router.HandleFunc("/release/boutonarbordetail/insert", services.InsertBoutonArborDetail)
	router.HandleFunc("/release/boutonarbordetail/delete", services.DeleteBoutonArbordetail)
	router.HandleFunc("/release/boutonarbordetail/query", services.QueryBoutonArborDetail)

	//collaborate service
	router.HandleFunc("/release/collaborate/getanoimage", services.GetAnoImage)
	router.HandleFunc("/release/collaborate/getanoneuron", services.GetAnoNeuron)
	router.HandleFunc("/release/collaborate/getano", services.GetAno)

	//给某个ano文件分配端口
	router.HandleFunc("/release/collaborate/inheritother", services.InheritOther)
	router.HandleFunc("/release/collaborate/createnewanofromzero", services.CreateFromZero)
	router.HandleFunc("/release/collaborate/createnewanofromother", services.CreateFromOther)

	//image service
	router.HandleFunc("/release/image/getimagelist", services.GetImageList)
	//获取裁剪后的图像文件.v3dpbd
	router.HandleFunc("/release/image/cropimage", services.CropImage)
	//获取裁剪后的eswc文件
	router.HandleFunc("/release/swc/cropswc", services.CropSWC)
	router.HandleFunc("/release/apo/cropapo", services.CropApo)
	//resource service
	router.HandleFunc("/release/musics", services.GetMusicList)
	router.HandleFunc("/release/updateapk", services.GetLatestApk)

	// game server
	//router.HandleFunc("/game/release/user/register", services.GameRegister)
	router.HandleFunc("/release/game/user/login", services.GameLogin)
	//router.HandleFunc("/game/release/user/uploadrecord", services.GameUpdataRecord)
	router.HandleFunc("/release/game/swc/bppoint/insert", services.InsertBranchingPoints)

	// 人脑组图像检查接口
	// 获取检查图像的list
	//http.HandleFunc("/release/image/gethumanimagelist", services.GetHumanImageList)
	// 上传人脑图像的检查结果
	//http.HandleFunc("/release/arbor/updatehumanarborresult", services.UpdateHumanArborResult)

	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8001", router))
}
