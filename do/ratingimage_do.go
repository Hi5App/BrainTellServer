package do

import (
	"BrainTellServer/utils"
	"crypto/md5"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
	"time"
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
	UploadTime                  string `xorm:"timestamp null 'UploadTime'"`                // 上传时间
}

type UserImageMap struct {
	Mu   sync.Mutex
	Data map[string][]UserData
}

type UserData struct {
	UserName string
	SendTime time.Time
}

var UserImageMapCachedData = UserImageMap{
	Mu:   sync.Mutex{},
	Data: make(map[string][]UserData),
}

func CleanupExpiredUserImageMapCachedData() {
	UserImageMapCachedData.Mu.Lock()
	defer UserImageMapCachedData.Mu.Unlock()

	threshold := time.Now().Add(-30 * time.Minute)
	for imageName, userDataList := range UserImageMapCachedData.Data {
		filteredUserDataList := userDataList[:0]
		for _, userData := range userDataList {
			if userData.SendTime.After(threshold) {
				filteredUserDataList = append(filteredUserDataList, userData)
			} else {
				fmt.Println("UserImageMapCachedData: ", imageName, " ", userData.UserName, " ", userData.SendTime, " is expired and removed.")
			}
		}
		if len(filteredUserDataList) == 0 {
			delete(UserImageMapCachedData.Data, imageName)
		} else {
			UserImageMapCachedData.Data[imageName] = filteredUserDataList
		}
	}
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
		RatedNum  int
	}
	var rawSql = `
SELECT t_rating_image.ImageName
FROM t_rating_image
		 LEFT JOIN (SELECT ImageName, COUNT(ImageName) AS RatedNum
					FROM t_rating_result
					GROUP BY ImageName) as NewTable ON t_rating_image.ImageName = NewTable.ImageName
WHERE RatedNum IS NULL OR RatedNum < 2
GROUP BY t_rating_image.ImageName
EXCEPT
SELECT DISTINCT t_rating_result.ImageName
FROM t_rating_result
WHERE t_rating_result.UserName = '%s'
	`
	querySql := fmt.Sprintf(rawSql, userName)
	err := utils.DB.SQL(querySql).Find(&images)
	if err != nil {
		return nil, err
	}

	for _, image := range images {
		// Check if the image has been sent to the user before
		if users, ok := UserImageMapCachedData.Data[image.ImageName]; ok {
			var sentBefore bool
			for _, user := range users {
				if user.UserName == userName {
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
		UserImageMapCachedData.Data[image.ImageName] = append(UserImageMapCachedData.Data[image.ImageName], UserData{
			UserName: userName,
			SendTime: time.Now(),
		})

		imageNameList = append(imageNameList, image.ImageName)

		// Limit the number of images according to the ImagesCount
		if int32(len(imageNameList)) >= imageCount {
			break
		}
	}

	return imageNameList, err
}

func InsertRatingResult(ratingResult TRatingResult) error {
	count, err := utils.DB.Table("t_rating_result").Where("ImageName = ? AND UserName = ?", ratingResult.ImageName, ratingResult.UserName).Count()
	if err != nil {
		return err
	}

	if count != 0 {
		newValues := map[string]interface{}{
			"ImageName":                   ratingResult.ImageName,
			"UserName":                    ratingResult.UserName,
			"RatingEnum":                  ratingResult.RatingEnum,
			"AdditionalRatingDescription": ratingResult.AdditionalRatingDescription,
			"UploadTime":                  ratingResult.UploadTime,
		}
		// 更新记录
		affected, err := utils.DB.Table("t_rating_result").Where("ImageName = ? AND UserName = ?", ratingResult.ImageName, ratingResult.UserName).Update(newValues)
		if err != nil {
			return err
		} else {
			// 处理更新成功
			fmt.Printf("更新了 %d 条记录\n", affected)
		}
		return nil
	} else {
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
}

func computeMD5(filePath string) (string, error) {
	bytes, err := ioutil.ReadFile(filePath)
	if err != nil {
		return "", err
	}

	hash := md5.Sum(bytes)
	return fmt.Sprintf("%x", hash), nil
}

func QueryRatingResult(userName string, startTime string, endTime string) ([]TRatingResult, error) {
	var results []TRatingResult
	session := utils.DB.Table("t_rating_result").Where("UserName = ? AND UploadTime > ? AND UploadTime < ?", userName, startTime, endTime)
	err := session.Find(&results)
	if err != nil {
		return nil, err
	}

	return results, nil
}
