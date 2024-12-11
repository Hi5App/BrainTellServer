package services

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"github.com/gorilla/mux"
	log "github.com/sirupsen/logrus"
	"net/http"
	"time"
)

type RatingResultRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`

	ImageName                   string `json:"ImageName"`
	SolutionName                string `json:"SolutionName"`
	RatingEnum                  string `json:"RatingEnum"`
	AdditionalRatingDescription string `json:"AdditionalRatingDescription"`
}

type GetRatingImageRequest struct {
	UserName string `json:"UserName"`

	Password string `json:"Password"`

	ImagesCount int32 `json:"ImagesCount"`
}

type RescanImageListRequest struct {
	UserName string `json:"UserName"`

	Password string `json:"Password"`

	FixedToken string `json:"FixedToken"`
}

type ImageNameListResponse struct {
	Status        string   `json:"Status"`
	ImageNameList []string `json:"ImageNameList"`
}

type ResponseStatus struct {
	Status string `json:"Status"`
}

type GetRatingResultRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`

	QuerySolutionName string `json:"QuerySolutionName"`
	QueryUserName     string `json:"QueryUserName"`
	QueryStartTime    string `json:"QueryStartTime"`
	QueryEndTime      string `json:"QueryEndTime"`
}

type RatingQueryResult struct {
	ImageName                   string `json:"ImageName"`
	SolutionName                string `json:"SolutionName"`
	UserName                    string `json:"UserName"`
	RatingEnum                  string `json:"RatingEnum"`
	AdditionalRatingDescription string `json:"AdditionalRatingDescription"`
	UploadTime                  string `json:"UploadTime"`
}

type GetRatingResultResponse struct {
	Status            string              `json:"Status"`
	RatingQueryResult []RatingQueryResult `json:"RatingQueryResult"`
}

type GetRatingUserNameRequest struct {
	UserName     string `json:"UserName"`
	Password     string `json:"Password"`
	SolutionName string `json:"SolutionName"`
}

type GetRatingUserNameResponse struct {
	Status         string   `json:"Status"`
	UserNameResult []string `json:"UserNameResult"`
}

type GetRatingSolutionRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`
}

type GetRatingSolutionResponse struct {
	Status         string     `json:"Status"`
	SolutionResult []Solution `json:"SolutionResult"`
}

type Solution struct {
	SolutionName   string `json:"SolutionName"`
	SolutionDetail string `json:"SolutionDetail"`
}

type UpdatedSolutionInfo struct {
	OldSolutionName string   `json:"OldSolutionName"`
	UpdatedSolution Solution `json:"UpdatedSolution"`
}

type AddRatingSolutionRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`

	AddedSolution []Solution `json:"AddedSolution"`
}

type UpdateRatingSolutionRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`

	UpdatedSolutionInfo []UpdatedSolutionInfo `json:"UpdatedSolutionInfo"`
}

type DeleteRatingSolutionRequest struct {
	UserName string `json:"UserName"`
	Password string `json:"Password"`

	DeletedSolution []string `json:"DeletedSolution"`
}

// BrainTellServerApiService is a service that implents the logic for the BrainTellServerApiServicer
// This service should implement the business logic for every endpoint for the BrainTellServerApi API.
// Include any external packages or services that will be required by this service.
type BrainTellServerApiService struct {
}

// NewBrainTellServerApiService creates a default api service
func NewBrainTellServerApiService() BrainTellServerApiServicer {
	return &BrainTellServerApiService{}
}

// GetRatingImageListPost - 获取打分图像列表
func (s *BrainTellServerApiService) GetRatingImageListPost(request GetRatingImageRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		var imageNameList []string
		return ImageNameListResponse{Status: result.Message, ImageNameList: imageNameList}, nil
	}
	if userMetaInfo.Password != request.Password {
		var imageNameList []string
		return ImageNameListResponse{Status: "password not correct", ImageNameList: imageNameList}, nil
	}

	do.UserImageMapCachedData.Mu.Lock()
	defer do.UserImageMapCachedData.Mu.Unlock()

	var imageNameList, err = do.GetRatingImageList(request.UserName, request.ImagesCount)
	if err != nil {
		return ImageNameListResponse{Status: err.Error(), ImageNameList: imageNameList}, nil
	}

	return ImageNameListResponse{Status: "OK", ImageNameList: imageNameList}, err
}

// UpdateRatingResultPost - 上传图像打分信息
func (s *BrainTellServerApiService) UpdateRatingResultPost(request RatingResultRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	do.UserImageMapCachedData.Mu.Lock()
	defer do.UserImageMapCachedData.Mu.Unlock()

	ratingResult := do.RatingResultV2{
		ImageName:                   request.ImageName,
		SolutionName:                request.SolutionName,
		UserName:                    request.UserName,
		RatingEnum:                  request.RatingEnum,
		AdditionalRatingDescription: request.AdditionalRatingDescription,
		UploadTime:                  time.Now().Format(time.DateTime),
	}

	err := do.InsertRatingResult(ratingResult)
	if err != nil {
		return ResponseStatus{Status: err.Error()}, nil
	}
	return ResponseStatus{Status: "OK"}, err
}

// RequestRescanImageListPost - 重新扫描图片列表
func (s *BrainTellServerApiService) RequestRescanImageListPost(request RescanImageListRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	do.UserImageMapCachedData.Mu.Lock()
	defer do.UserImageMapCachedData.Mu.Unlock()

	do.RescanImageAndUpdateDB()

	return ResponseStatus{Status: "OK"}, nil
}

// GetRatingImageFileGet - 获取打分的图片文件
func HandleRatingStaticImageFile(router *mux.Router) {
	fs := http.FileServer(http.Dir(utils.RatingImagePath))
	router.PathPrefix("/release/GetRatingImageFile").Handler(http.StripPrefix("/release/GetRatingImageFile", fs))
}

func HandleRatingStaticImageFileCompressed() {
	//var imagePath = utils.RatingImagePath
	//
	//f, err := os.Open("sid.jpg")
	//
	//if err != nil {
	//	log.Println(err.Error())
	//}
	//
	//defer func(f *os.File) {
	//	err := f.Close()
	//	if err != nil {
	//		log.Println(err.Error())
	//	}
	//}(f)
	//
	//reader := bufio.NewReader(f)
	//buf := make([]byte, 256)
	//
	//for {
	//	_, err := reader.Read(buf)
	//
	//	if err != nil {
	//		if err != io.EOF {
	//			fmt.Println(err)
	//		}
	//		break
	//	}
	//
	//	fmt.Printf("%s", hex.Dump(buf))
	//}

}

func InitializeScheduleExpiredImageList() {
	ticker := time.NewTicker(10 * time.Minute)
	go func() {
		for {
			select {
			case <-ticker.C:
				do.CleanupExpiredUserImageMapCachedData()
			}
		}
	}()
}

func (s *BrainTellServerApiService) GetRatingResultPost(request GetRatingResultRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		var imageNameList []string
		return ImageNameListResponse{Status: result.Message, ImageNameList: imageNameList}, nil
	}
	if userMetaInfo.Password != request.Password {
		var imageNameList []string
		return ImageNameListResponse{Status: "password not correct", ImageNameList: imageNameList}, nil
	}

	query := do.QueryRatingResultInfo{
		SolutionName: request.QuerySolutionName,
		UserName:     request.QueryUserName,
		StartTime:    request.QueryStartTime,
		EndTime:      request.QueryEndTime,
	}
	results, err := do.QueryRatingResult(query)
	if err != nil {
		var ratingQueryResult []RatingQueryResult
		return GetRatingResultResponse{Status: err.Error(), RatingQueryResult: ratingQueryResult}, nil
	}
	var ratingQueryResult []RatingQueryResult
	for _, tmpResult := range results {
		ratingQueryResult = append(ratingQueryResult, RatingQueryResult{
			ImageName:                   tmpResult.ImageName,
			SolutionName:                tmpResult.SolutionName,
			RatingEnum:                  tmpResult.RatingEnum,
			UserName:                    tmpResult.UserName,
			AdditionalRatingDescription: tmpResult.AdditionalRatingDescription,
			UploadTime:                  tmpResult.UploadTime,
		})
	}
	return GetRatingResultResponse{Status: "OK", RatingQueryResult: ratingQueryResult}, nil
}

func (s *BrainTellServerApiService) GetRatingUserNamePost(request GetRatingUserNameRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	usernames, err := do.GetRatingUserName(request.SolutionName)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Get rating username",
			"err":   err,
		}).Infof("Failed")
		return GetRatingUserNameResponse{Status: err.Error(), UserNameResult: nil}, nil
	}

	return GetRatingUserNameResponse{Status: "OK", UserNameResult: usernames}, err
}

func (s *BrainTellServerApiService) GetRatingSolutionPost(request GetRatingSolutionRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	solutions, err := do.GetRatingSolution()
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Get rating solution",
			"err":   err,
		}).Infof("Failed")
		var SolutionResult []Solution
		return GetRatingSolutionResponse{Status: err.Error(), SolutionResult: SolutionResult}, nil
	}

	var SolutionResult []Solution
	for _, solution := range solutions {
		SolutionResult = append(SolutionResult, Solution{
			SolutionName:   solution.SolutionName,
			SolutionDetail: solution.SolutionDetail,
		})
	}

	return GetRatingSolutionResponse{Status: "OK", SolutionResult: SolutionResult}, err
}

func (s *BrainTellServerApiService) AddRatingSolutionPost(request AddRatingSolutionRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	var addedData []do.AddRatingSolutionInfo

	for _, addedSolution := range request.AddedSolution {
		addedData = append(addedData, do.AddRatingSolutionInfo{
			SolutionName:   addedSolution.SolutionName,
			SolutionDetail: addedSolution.SolutionDetail,
		})
	}

	err := do.AddRatingSolution(addedData)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Add rating solution",
			"err":   err,
		}).Infof("Failed")
		return ResponseStatus{Status: err.Error()}, nil
	}
	return ResponseStatus{Status: "OK"}, err
}

func (s *BrainTellServerApiService) UpdateRatingSolutionPost(request UpdateRatingSolutionRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	var updatedData []do.UpdateRatingSolutionInfo

	for _, updatedSolutionInfo := range request.UpdatedSolutionInfo {
		updatedData = append(updatedData, do.UpdateRatingSolutionInfo{
			OldSolutionName: updatedSolutionInfo.OldSolutionName,
			NewSolutionName: updatedSolutionInfo.UpdatedSolution.SolutionName,
			SolutionDetail:  updatedSolutionInfo.UpdatedSolution.SolutionDetail,
		})
	}

	err := do.UpdateRatingSolution(updatedData)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Update rating solution",
			"err":   err,
		}).Infof("Failed")
		return ResponseStatus{Status: err.Error()}, nil
	}
	return ResponseStatus{Status: "OK"}, err
}

func (s *BrainTellServerApiService) DeleteRatingSolutionPost(request DeleteRatingSolutionRequest) (interface{}, error) {
	var userMetaInfo = models.UserMetaInfoV1{}
	result := do.QueryUser(&userMetaInfo, request.UserName)

	if !result.Status {
		return ResponseStatus{Status: result.Message}, nil
	}
	if userMetaInfo.Password != request.Password {
		return ResponseStatus{Status: "password not correct"}, nil
	}

	err := do.DeleteRatingSolution(request.DeletedSolution)
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Delete rating solution",
			"err":   err,
		}).Infof("Failed")
		return ResponseStatus{Status: err.Error()}, nil
	}
	return ResponseStatus{Status: "OK"}, err
}
