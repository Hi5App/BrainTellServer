package do

import (
	"BrainTellServer/utils"
	"crypto/md5"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
)

type TRatingImage struct {
	ID        int64  `xorm:"pk autoincr 'ID'"`                  // 主键，自增
	ImageName string `xorm:"varchar(256) not null 'ImageName'"` // 图像名称
	ImageMD5  string `xorm:"varchar(256) 'ImageMD5'"`           // 图像MD5
}

type TRatingResult struct {
	ID                          int64  `xorm:"pk autoincr 'ID'"`                           // 主键，自增
	ImageName                   string `xorm:"varchar(256) not null 'ImageName'"`          // 图像名称
	UserName                    string `xorm:"varchar(256) 'UserName'"`                    // 用户名
	RatingEnum                  string `xorm:"varchar(256) 'RatingEnum'"`                  // 评分枚举
	AdditionalRatingDescription string `xorm:"varchar(256) 'AdditionalRatingDescription'"` // 附加评分描述
}

type UserImageMap struct {
	Mu   sync.Mutex
	Data map[string][]string
}

var UserImageMapCachedData = UserImageMap{
	Mu:   sync.Mutex{},
	Data: make(map[string][]string),
}

func RescanImageAndUpdateDB() {
	var err = filepath.Walk(utils.RatingImagePath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if !info.IsDir() {
			imageName := info.Name()
			imageMD5, err := computeMD5(path)
			if err != nil {
				return err
			}

			has, err := utils.DB.Get(&TRatingImage{ImageName: imageName})
			if err != nil {
				return err
			}

			if !has {
				_, err = utils.DB.Insert(&TRatingImage{ImageName: imageName, ImageMD5: imageMD5})
				if err != nil {
					return err
				}
				fmt.Printf("Inserted image %s with MD5 %s\n", imageName, imageMD5)
			}
		}

		return nil
	})

	if err != nil {
		fmt.Println("Error:", err)
	}
}

func GetRatingImageList(userName string, imageCount int32) ([]string, error) {
	var imageNameList []string

	var images []struct {
		ImageName string `xorm:"ImageName"`
	}
	err := utils.DB.Table("t_rating_image").Cols("ImageName").Find(&images)
	if err != nil {
		return nil, err
	}

	for _, image := range images {
		var count int64
		count, err := utils.DB.Table("t_rating_result").Where("ImageName = ?", image.ImageName).Count()
		if err != nil {
			return nil, err
		}
		if count >= 2 {
			// If the image has been rated by two users already, skip it
			continue
		}

		// Check if the image has been sent to the user before
		if users, ok := UserImageMapCachedData.Data[image.ImageName]; ok {
			var sentBefore bool
			for _, user := range users {
				if user == userName {
					sentBefore = true
					break
				}
			}
			if sentBefore || len(users) >= 2 {
				// If the image has been sent to the user before, or has been sent to two users already, skip it
				continue
			}
		}

		// Add the user to the list of users who have received the image
		UserImageMapCachedData.Data[image.ImageName] = append(UserImageMapCachedData.Data[image.ImageName], userName)

		imageNameList = append(imageNameList, image.ImageName)

		// Limit the number of images according to the ImagesCount
		if int32(len(imageNameList)) >= imageCount {
			break
		}
	}

	return imageNameList, err
}

func InsertRatingResult(ratingResult TRatingResult) error {
	// 插入新的评分结果到数据库
	_, err := utils.DB.Insert(&ratingResult)
	if err != nil {
		return fmt.Errorf("failed to insert rating result: %v", err)
	}

	// 检查UserImageMapCachedData
	usernames, ok := UserImageMapCachedData.Data[ratingResult.ImageName]
	if ok && len(usernames) > 2 {
		delete(UserImageMapCachedData.Data, ratingResult.ImageName)
	}
	return nil
}

func computeMD5(filePath string) (string, error) {
	bytes, err := ioutil.ReadFile(filePath)
	if err != nil {
		return "", err
	}

	hash := md5.Sum(bytes)
	return fmt.Sprintf("%x", hash), nil
}
