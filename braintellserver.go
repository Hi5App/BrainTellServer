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
	//services.SendPerformance()

	//http.HandleFunc("/test/", func(w http.ResponseWriter, r *http.Request) {
	//	fmt.Fprintln(w, "test")
	//})
	//user service
	http.HandleFunc("/test/user/register", services.Register)
	http.HandleFunc("/test/user/login", services.Login)
	http.HandleFunc("/test/user/setuserscore", services.SetUserScore)       //todo
	http.HandleFunc("/test/user/updatepasswd", services.UpdatePasswd)       //todo
	http.HandleFunc("/test/user/resetpasswd", services.ResetPasswd)         //todo
	http.HandleFunc("/test/user/registernetease", services.RegisterNetease) //todo
	http.HandleFunc("/test/user/getuserperformance", services.GetUserPerformance)
	//add soma service
	http.HandleFunc("/test/soma/getpotentiallocation", services.GetPotentialSomaLocation)
	http.HandleFunc("/test/soma/getsomalist", services.GetSomaList)
	http.HandleFunc("/test/soma/updatesomalist", services.UpdateSomaList)
	//check arbor service
	http.HandleFunc("/test/arbor/getarbor", services.GetArbor)
	http.HandleFunc("/test/arbor/queryarborresult", services.QueryArborResult)
	http.HandleFunc("/test/arbor/updatearborresult", services.UpdateArborResult)
	//增加一些
	http.HandleFunc("/test/arbordetail/insert", services.InsertArborDetail)
	//删一些
	http.HandleFunc("/test/arbordetail/delete", services.DeleteArbordetail)
	//查一个
	http.HandleFunc("/test/arbordetail/query", services.QueryArborDetail)

	//// bouton check service
	http.HandleFunc("/test/arbor/getboutonarbor", services.GetBoutonArbor)
	http.HandleFunc("/test/arbor/getboutonarborimage", services.GetBoutonArborImage) // 临时的bouton获取服务，直接获取bouton的image文件
	http.HandleFunc("/test/arbor/updateboutonarborresult", services.UpdateBoutonArborResult)
	http.HandleFunc("/test/arbor/queryboutonarborresult", services.QueryBoutonArborResult)

	//// bouton detail的相关操作
	http.HandleFunc("/test/boutonarbordetail/insert", services.InsertBoutonArborDetail)
	http.HandleFunc("/test/boutonarbordetail/delete", services.DeleteBoutonArbordetail)
	http.HandleFunc("/test/boutonarbordetail/query", services.QueryBoutonArborDetail)

	//collaborate service
	http.HandleFunc("/test/collaborate/getanoimage", services.GetAnoImage)
	http.HandleFunc("/test/collaborate/getanoneuron", services.GetAnoNeuron)
	http.HandleFunc("/test/collaborate/getano", services.GetAno)

	//给某个ano文件分配端口
	http.HandleFunc("/test/collaborate/inheritother", services.InheritOther)
	http.HandleFunc("/test/collaborate/createnewanofromzero", services.CreateFromZero)
	http.HandleFunc("/test/collaborate/createnewanofromother", services.CreateFromOther)

	//image service
	http.HandleFunc("/test/image/getimagelist", services.GetImageList)
	//获取裁剪后的图像文件.v3dpbd
	http.HandleFunc("/test/image/cropimage", services.CropImage)
	//获取裁剪后的eswc文件
	http.HandleFunc("/test/swc/cropswc", services.CropSWC)
	http.HandleFunc("/test/apo/cropapo", services.CropApo)
	//resource service
	http.HandleFunc("/test/musics", services.GetMusicList)
	http.HandleFunc("/test/updateapk", services.GetLatestApk)

	// game server
	//http.HandleFunc("/game/test/user/register", services.GameRegister)
	http.HandleFunc("/test/game/user/login", services.GameLogin)
	//http.HandleFunc("/game/test/user/uploadrecord", services.GameUpdataRecord)
	http.HandleFunc("/test/game/swc/bppoint/insert", services.InsertBranchingPoints)

	// 人脑组图像检查接口
	// 获取检查图像的list
	//http.HandleFunc("/test/image/gethumanimagelist", services.GetHumanImageList)
	// 上传人脑图像的检查结果
	//http.HandleFunc("/test/arbor/updatehumanarborresult", services.UpdateHumanArborResult)

	log.WithFields(log.Fields{
		"event": "start server",
	}).Fatal(http.ListenAndServe(":8001", nil))
}
