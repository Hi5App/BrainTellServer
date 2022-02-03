package utils

type RequestParam interface {
	String() string
	FromJsonString(jsonstr string) (RequestParam, error)
}
