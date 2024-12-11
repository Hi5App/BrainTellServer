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
			"/release/GetRatingImageList",
			c.GetRatingImageListPost,
		},
		{
			"RequestRescanImageListPost",
			strings.ToUpper("Post"),
			"/release/RequestRescanImageList",
			c.RequestRescanImageListPost,
		},
		{
			"UpdateRatingResultPost",
			strings.ToUpper("Post"),
			"/release/UpdateRatingResult",
			c.UpdateRatingResultPost,
		},
		{
			"GetRatingResultPost",
			strings.ToUpper("Post"),
			"/release/GetRatingResult",
			c.GetRatingResultPost,
		},
		{
			"GetRatingUserNamePost",
			strings.ToUpper("Post"),
			"/release/GetRatingUserName",
			c.GetRatingUserNamePost,
		},
		{
			"GetRatingSolutionPost",
			strings.ToUpper("Post"),
			"/release/GetRatingSolution",
			c.GetRatingSolutionPost,
		},
		{
			"AddRatingSolutionPost",
			strings.ToUpper("Post"),
			"/release/AddRatingSolution",
			c.AddRatingSolutionPost,
		},
		{
			"UpdateRatingSolutionPost",
			strings.ToUpper("Post"),
			"/release/UpdateRatingSolution",
			c.UpdateRatingSolutionPost,
		},
		{
			"DeleteRatingSolutionPost",
			strings.ToUpper("Post"),
			"/release/DeleteRatingSolution",
			c.DeleteRatingSolutionPost,
		},
	}
}

// GetRatingImageListPost - 获取打分图像列表
func (c *BrainTellServerApiController) GetRatingImageListPost(w http.ResponseWriter, r *http.Request) {
	request := &GetRatingImageRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		w.WriteHeader(400)
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
		w.WriteHeader(400)
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
		w.WriteHeader(400)
		return
	}

	result, err := c.service.RequestRescanImageListPost(*request)
	if err != nil {
		w.WriteHeader(500)
		return
	}

	EncodeJSONResponse(result, nil, w)
}

func (c *BrainTellServerApiController) GetRatingResultPost(w http.ResponseWriter, r *http.Request) {
	request := &GetRatingResultRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		w.WriteHeader(400)
		return
	}

	result, err := c.service.GetRatingResultPost(*request)
	if err != nil {
		w.WriteHeader(500)
		return
	}

	EncodeJSONResponse(result, nil, w)
}

func (c *BrainTellServerApiController) GetRatingUserNamePost(w http.ResponseWriter, r *http.Request) {
	request := &GetRatingUserNameRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Failed to decode JSON", http.StatusBadRequest)
		return
	}

	result, _ := c.service.GetRatingUserNamePost(*request)

	err := EncodeJSONResponse(result, nil, w)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

func (c *BrainTellServerApiController) GetRatingSolutionPost(w http.ResponseWriter, r *http.Request) {
	request := &GetRatingSolutionRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Failed to decode JSON", http.StatusBadRequest)
		return
	}

	result, _ := c.service.GetRatingSolutionPost(*request)

	err := EncodeJSONResponse(result, nil, w)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

func (c *BrainTellServerApiController) AddRatingSolutionPost(w http.ResponseWriter, r *http.Request) {
	request := &AddRatingSolutionRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Failed to decode JSON", http.StatusBadRequest)
		return
	}

	if request.AddedSolution == nil || len(request.AddedSolution) == 0 {
		http.Error(w, "No added solution", http.StatusBadRequest)
		return
	}

	result, _ := c.service.AddRatingSolutionPost(*request)

	err := EncodeJSONResponse(result, nil, w)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

func (c *BrainTellServerApiController) UpdateRatingSolutionPost(w http.ResponseWriter, r *http.Request) {
	request := &UpdateRatingSolutionRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Failed to decode JSON", http.StatusBadRequest)
		return
	}

	if request.UpdatedSolutionInfo == nil || len(request.UpdatedSolutionInfo) == 0 {
		http.Error(w, "No updated solution", http.StatusBadRequest)
		return
	}

	result, _ := c.service.UpdateRatingSolutionPost(*request)

	err := EncodeJSONResponse(result, nil, w)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

func (c *BrainTellServerApiController) DeleteRatingSolutionPost(w http.ResponseWriter, r *http.Request) {
	request := &DeleteRatingSolutionRequest{}
	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		http.Error(w, "Failed to decode JSON", http.StatusBadRequest)
		return
	}

	if request.DeletedSolution == nil || len(request.DeletedSolution) == 0 {
		http.Error(w, "No updated solution", http.StatusBadRequest)
		return
	}

	result, _ := c.service.DeleteRatingSolutionPost(*request)

	err := EncodeJSONResponse(result, nil, w)
	if err != nil {
		http.Error(w, "Failed to encode JSON", http.StatusInternalServerError)
	}
}

// BrainTellServerApiRouter defines the required methods for binding the api requests to a responses for the BrainTellServerApi
// The BrainTellServerApiRouter implementation should parse necessary information from the http request,
// pass the data to a BrainTellServerApiServicer to perform the required actions, then write the service results to the http response.
type BrainTellServerApiRouter interface {
	GetRatingImageListPost(http.ResponseWriter, *http.Request)
	RequestRescanImageListPost(http.ResponseWriter, *http.Request)
	UpdateRatingResultPost(http.ResponseWriter, *http.Request)
	GetRatingResultPost(http.ResponseWriter, *http.Request)
	GetRatingUserNamePost(http.ResponseWriter, *http.Request)

	GetRatingSolutionPost(http.ResponseWriter, *http.Request)
	AddRatingSolutionPost(http.ResponseWriter, *http.Request)
	UpdateRatingSolutionPost(http.ResponseWriter, *http.Request)
	DeleteRatingSolutionPost(http.ResponseWriter, *http.Request)
}

// BrainTellServerApiServicer defines the api actions for the BrainTellServerApi service
// This interface intended to stay up to date with the openapi yaml used to generate it,
// while the service implementation can ignored with the .openapi-generator-ignore file
// and updated with the logic required for the API.
type BrainTellServerApiServicer interface {
	GetRatingImageListPost(GetRatingImageRequest) (interface{}, error)
	RequestRescanImageListPost(RescanImageListRequest) (interface{}, error)
	UpdateRatingResultPost(RatingResultRequest) (interface{}, error)
	GetRatingResultPost(GetRatingResultRequest) (interface{}, error)
	GetRatingUserNamePost(GetRatingUserNameRequest) (interface{}, error)

	GetRatingSolutionPost(GetRatingSolutionRequest) (interface{}, error)
	AddRatingSolutionPost(AddRatingSolutionRequest) (interface{}, error)
	UpdateRatingSolutionPost(UpdateRatingSolutionRequest) (interface{}, error)
	DeleteRatingSolutionPost(DeleteRatingSolutionRequest) (interface{}, error)
}
