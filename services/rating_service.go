package services

import (
	"errors"
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
	ImageNameList []string `json:"ImageNameList"`
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

// GetRatingImageFileGet - 获取打分的图片文件
func (s *BrainTellServerApiService) GetRatingImageFileGet(imageName string) (interface{}, error) {
	// TODO - update GetRatingImageFileGet with the required logic for this service method.
	// Add api_brain_tell_server_service.go to the .openapi-generator-ignore to avoid overwriting this service implementation when updating open api generation.
	return nil, errors.New("service method 'GetRatingImageFileGet' not implemented")
}

// GetRatingImageListPost - 获取打分图像列表
func (s *BrainTellServerApiService) GetRatingImageListPost(inlineObject GetRatingImageRequest) (interface{}, error) {
	// TODO - update GetRatingImageListPost with the required logic for this service method.
	// Add api_brain_tell_server_service.go to the .openapi-generator-ignore to avoid overwriting this service implementation when updating open api generation.
	return nil, errors.New("service method 'GetRatingImageListPost' not implemented")
}

// UpdateRatingResultPost - 上传图像打分信息
func (s *BrainTellServerApiService) UpdateRatingResultPost(inlineObject1 RatingResultRequest) (interface{}, error) {
	// TODO - update UpdateRatingResultPost with the required logic for this service method.
	// Add api_brain_tell_server_service.go to the .openapi-generator-ignore to avoid overwriting this service implementation when updating open api generation.
	return nil, errors.New("service method 'UpdateRatingResultPost' not implemented")
}

// RequestRescanImageListPost - 重新扫描图片列表
func (s *BrainTellServerApiService) RequestRescanImageListPost(inlineObject2 RescanImageListRequest) (interface{}, error) {
	// TODO - update RequestRescanImageListPost with the required logic for this service method.
	// Add api_brain_tell_server_service.go to the .openapi-generator-ignore to avoid overwriting this service implementation when updating open api generation.
	return nil, errors.New("service method 'RequestRescanImageListPost' not implemented")
}
