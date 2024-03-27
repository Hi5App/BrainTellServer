package services

import (
	"BrainTellServer/do"
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"github.com/gorilla/mux"
	"net/http"
)

type RatingResultRequest struct {
	UserName string `json:"UserName"`

	Password string `json:"Password"`

	ImageName string `json:"ImageName"`

	RatingEnum string `json:"RatingEnum"`

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

	// 创建一个新的评分结果对象
	ratingResult := do.TRatingResult{
		ImageName:                   request.ImageName,
		UserName:                    request.UserName,
		RatingEnum:                  request.RatingEnum,
		AdditionalRatingDescription: request.AdditionalRatingDescription,
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
	router.PathPrefix("/dynamic/GetRatingImageFile").Handler(http.StripPrefix("/dynamic/GetRatingImageFile", fs))
}
