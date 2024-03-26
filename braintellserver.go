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

	// connect to mongodb for user data management
	createInfo := utils.MongoDbConnectionCreateInfo{
		Host:     "127.0.0.1",
		Port:     27017,
		User:     "defaultuser",
		Password: "defaultpassword",
	}
	utils.InitializeMongodbConnection(createInfo)

	services.SendPerformance()

	BrainTellServerApiService := services.NewBrainTellServerApiService()
	BrainTellServerApiController := services.NewBrainTellServerApiController(BrainTellServerApiService)

	router := services.NewRouter(BrainTellServerApiController)
	//http.HandleFunc("/dynamic/", func(w http.ResponseWriter, r *http.Request) {
	//	fmt.Fprintln(w, "dynamic")
	//})
	//user service
	router.HandleFunc("/dynamic/user/register", services.Register)
	router.HandleFunc("/dynamic/user/login", services.Login)
	router.HandleFunc("/dynamic/user/setuserscore", services.SetUserScore)       //todo
	router.HandleFunc("/dynamic/user/updatepasswd", services.UpdatePasswd)       //todo
	router.HandleFunc("/dynamic/user/resetpasswd", services.ResetPasswd)         //todo
	router.HandleFunc("/dynamic/user/registernetease", services.RegisterNetease) //todo
	router.HandleFunc("/dynamic/user/getuserperformance", services.GetUserPerformance)
	//add soma service
	router.HandleFunc("/dynamic/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	router.HandleFunc("/dynamic/soma/getsomalist", services.GetSomaList)
	router.HandleFunc("/dynamic/soma/updatesomalist", services.UpdateSomaList)
	//check arbor service
	router.HandleFunc("/dynamic/arbor/getarbor", services.GetArbor)
	router.HandleFunc("/dynamic/arbor/queryarborresult", services.QueryArborResult)
	router.HandleFunc("/dynamic/arbor/updatearborresult", services.UpdateArborResult)
	//增加一些
	router.HandleFunc("/dynamic/arbordetail/insert", services.InsertArborDetail)
	//删一些
	router.HandleFunc("/dynamic/arbordetail/delete", services.DeleteArbordetail)
	//查一个
	router.HandleFunc("/dynamic/arbordetail/query", services.QueryArborDetail)

	//// bouton check service
	router.HandleFunc("/dynamic/arbor/getboutonarbor", services.GetBoutonArbor)
	router.HandleFunc("/dynamic/arbor/getboutonarborimage", services.GetBoutonArborImage) // 临时的bouton获取服务，直接获取bouton的image文件
	router.HandleFunc("/dynamic/arbor/updateboutonarborresult", services.UpdateBoutonArborResult)
	router.HandleFunc("/dynamic/arbor/queryboutonarborresult", services.QueryBoutonArborResult)

	//// bouton detail的相关操作
	router.HandleFunc("/dynamic/boutonarbordetail/insert", services.InsertBoutonArborDetail)
	router.HandleFunc("/dynamic/boutonarbordetail/delete", services.DeleteBoutonArbordetail)
	router.HandleFunc("/dynamic/boutonarbordetail/query", services.QueryBoutonArborDetail)

	//collaborate service
	router.HandleFunc("/dynamic/collaborate/getanoimage", services.GetAnoImage)
	router.HandleFunc("/dynamic/collaborate/getanoneuron", services.GetAnoNeuron)
	router.HandleFunc("/dynamic/collaborate/getano", services.GetAno)

	//给某个ano文件分配端口
	router.HandleFunc("/dynamic/collaborate/inheritother", services.InheritOther)
	router.HandleFunc("/dynamic/collaborate/createnewanofromzero", services.CreateFromZero)
	router.HandleFunc("/dynamic/collaborate/createnewanofromother", services.CreateFromOther)

	//image service
	router.HandleFunc("/dynamic/image/getimagelist", services.GetImageList)
	//获取裁剪后的图像文件.v3dpbd
	router.HandleFunc("/dynamic/image/cropimage", services.CropImage)
	//获取裁剪后的eswc文件
	router.HandleFunc("/dynamic/swc/cropswc", services.CropSWC)
	router.HandleFunc("/dynamic/apo/cropapo", services.CropApo)
	//resource service
	router.HandleFunc("/dynamic/musics", services.GetMusicList)
	router.HandleFunc("/dynamic/updateapk", services.GetLatestApk)

	// game server
	//router.HandleFunc("/game/dynamic/user/register", services.GameRegister)
	router.HandleFunc("/dynamic/game/user/login", services.GameLogin)
	//router.HandleFunc("/game/dynamic/user/uploadrecord", services.GameUpdataRecord)
	router.HandleFunc("/dynamic/game/swc/bppoint/insert", services.InsertBranchingPoints)

	// 人脑组图像检查接口
	// 获取检查图像的list
	//http.HandleFunc("/dynamic/image/gethumanimagelist", services.GetHumanImageList)
	// 上传人脑图像的检查结果
	//http.HandleFunc("/dynamic/arbor/updatehumanarborresult", services.UpdateHumanArborResult)

	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8000", router))
}
