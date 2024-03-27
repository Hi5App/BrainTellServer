package services

import (
	"encoding/json"
	"net/http"
	"strings"
)

// A BrainTellServerApiController binds http requests to an api service and writes the service results to the http response
type BrainTellServerApiController struct {
	service BrainTellServerApiServicer
}

// NewBrainTellServerApiController creates a default api controller
func NewBrainTellServerApiController(s BrainTellServerApiServicer) Router {
	return &BrainTellServerApiController{service: s}
}

// Routes returns all of the api route for the BrainTellServerApiController
func (c *BrainTellServerApiController) Routes() Routes {
	return Routes{
		{
			"GetRatingImageListPost",
			strings.ToUpper("Post"),
			"/dynamic/GetRatingImageList",
			c.GetRatingImageListPost,
		},
		{
			"RequestRescanImageListPost",
			strings.ToUpper("Post"),
			"/dynamic/RequestRescanImageList",
			c.RequestRescanImageListPost,
		},
		{
			"UpdateRatingResultPost",
			strings.ToUpper("Post"),
			"/dynamic/UpdateRatingResult",
			c.UpdateRatingResultPost,
		},
	}
}

// GetRatingImageListPost - 获取打分图像列表
func (c *BrainTellServerApiController) GetRatingImageListPost(w http.ResponseWriter, r *http.Request) {
	request := &GetRatingImageRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		w.WriteHeader(500)
		return
	}

	result, err := c.service.GetRatingImageListPost(*request)
	if err != nil {
		w.WriteHeader(500)
		return
	}

	EncodeJSONResponse(result, nil, w)
}

// UpdateRatingResultPost - 上传图像打分信息
func (c *BrainTellServerApiController) UpdateRatingResultPost(w http.ResponseWriter, r *http.Request) {
	request := &RatingResultRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		w.WriteHeader(500)
		return
	}

	result, err := c.service.UpdateRatingResultPost(*request)
	if err != nil {
		w.WriteHeader(500)
		return
	}

	EncodeJSONResponse(result, nil, w)
}

// RequestRescanImageListPost - 重新扫描图片列表
func (c *BrainTellServerApiController) RequestRescanImageListPost(w http.ResponseWriter, r *http.Request) {
	request := &RescanImageListRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		w.WriteHeader(500)
		return
	}

	result, err := c.service.RequestRescanImageListPost(*request)
	if err != nil {
		w.WriteHeader(500)
		return
	}

	EncodeJSONResponse(result, nil, w)
}

// BrainTellServerApiRouter defines the required methods for binding the api requests to a responses for the BrainTellServerApi
// The BrainTellServerApiRouter implementation should parse necessary information from the http request,
// pass the data to a BrainTellServerApiServicer to perform the required actions, then write the service results to the http response.
type BrainTellServerApiRouter interface {
	GetRatingImageListPost(http.ResponseWriter, *http.Request)
	RequestRescanImageListPost(http.ResponseWriter, *http.Request)
	UpdateRatingResultPost(http.ResponseWriter, *http.Request)
}

// BrainTellServerApiServicer defines the api actions for the BrainTellServerApi service
// This interface intended to stay up to date with the openapi yaml used to generate it,
// while the service implementation can ignored with the .openapi-generator-ignore file
// and updated with the logic required for the API.
type BrainTellServerApiServicer interface {
	GetRatingImageListPost(GetRatingImageRequest) (interface{}, error)
	RequestRescanImageListPost(RescanImageListRequest) (interface{}, error)
	UpdateRatingResultPost(RatingResultRequest) (interface{}, error)
}
