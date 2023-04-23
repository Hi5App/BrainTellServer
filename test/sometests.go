package test

import (
	"BrainTellServer/utils"
	"fmt"
	"strconv"
)

func testImageListSorts() {
	images := []string{"12345", "2301322", "00239", "012445", "00015", "00023", "02028", "00044", "01015", "00702"}
	fmt.Println(images)

	var numbers []int
	for _, value := range images {
		var number int
		number, _ = strconv.Atoi(value)
		numbers = append(numbers, number)
	}
	fmt.Println(numbers)

	var index []int = utils.Sort(numbers)

	var sortedImages []string
	for _, value := range index {
		sortedImages = append(sortedImages, images[value])
	}
	fmt.Println(sortedImages)
}

//func main() {
//	//testImageListSorts()
//}
