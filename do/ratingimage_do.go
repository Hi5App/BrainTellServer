package do

import (
	"BrainTellServer/utils"
	"crypto/md5"
	"fmt"
	log "github.com/sirupsen/logrus"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync"
	"time"
	"xorm.io/xorm"
)

type TRatingImage struct {
	ID        int64  `xorm:"pk autoincr 'ID'"`                  // 主键，自增
	ImageName string `xorm:"varchar(256) not null 'ImageName'"` // 图像名称
	ImageMD5  string `xorm:"varchar(256) 'ImageMD5'"`           // 图像MD5
}

type TRatingResult struct {
	ID                          int64  `xorm:"pk autoincr 'ID'"`                           // 主键，自增
	ImageName                   string `xorm:"varchar(256) not null 'ImageName'"`          // 图像名称
	SolutionID                  int64  `xorm:"int not null 'SolutionID'"`                  // 分类方案ID
	UserName                    string `xorm:"varchar(256) 'UserName'"`                    // 用户名
	RatingEnum                  string `xorm:"varchar(256) 'RatingEnum'"`                  // 评分枚举
	AdditionalRatingDescription string `xorm:"varchar(256) 'AdditionalRatingDescription'"` // 附加评分描述
	UploadTime                  string `xorm:"timestamp null 'UploadTime'"`                // 上传时间
}

type RatingResultV2 struct {
	ImageName                   string
	SolutionName                string
	UserName                    string
	RatingEnum                  string
	AdditionalRatingDescription string
	UploadTime                  string
}

type TRatingSolution struct {
	ID             int64
	SolutionName   string
	SolutionDetail string
	IsDeleted      bool
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

type QueryRatingResultInfo struct {
	SolutionName string
	UserName     string
	StartTime    string
	EndTime      string
}

type UpdateRatingSolutionInfo struct {
	OldSolutionName string
	NewSolutionName string
	SolutionDetail  string
}

type AddRatingSolutionInfo struct {
	SolutionName   string
	SolutionDetail string
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
	//	var rawSql = `
	//SELECT t_rating_image.ImageName
	//FROM t_rating_image
	//		 LEFT JOIN (SELECT ImageName, COUNT(ImageName) AS RatedNum
	//					FROM t_rating_result
	//					GROUP BY ImageName) as NewTable ON t_rating_image.ImageName = NewTable.ImageName
	//WHERE RatedNum IS NULL OR RatedNum < 2
	//GROUP BY t_rating_image.ImageName
	//EXCEPT
	//SELECT DISTINCT t_rating_result.ImageName
	//FROM t_rating_result
	//WHERE t_rating_result.UserName = '%s'
	//	`
	// mysql8.0不支持EXCEPT，所以改成下面的写法
	var rawSql = `
SELECT t_rating_image.ImageName
FROM t_rating_image
         LEFT JOIN (SELECT ImageName, COUNT(ImageName) AS RatedNum
                    FROM t_rating_result
                    GROUP BY ImageName) as NewTable ON t_rating_image.ImageName = NewTable.ImageName
         LEFT JOIN (SELECT DISTINCT ImageName
                    FROM t_rating_result
                    WHERE UserName = '%s') as UserImages ON t_rating_image.ImageName = UserImages.ImageName
WHERE (NewTable.RatedNum IS NULL OR NewTable.RatedNum < 2)
  AND UserImages.ImageName IS NULL
GROUP BY t_rating_image.ImageName;
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

func InsertRatingResult(ratingResult RatingResultV2) error {
	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "insert rating result",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "insert rating result",
			"err":   err,
		}).Infof("Failed")
	}

	var solutionID int64
	has, err := session.Table("t_rating_solution").Cols("ID").Where("SolutionName = ?", ratingResult.SolutionName).Get(&solutionID)
	if err != nil {
		return err
	}
	if !has {
		return nil
	}

	count, err := session.Table("t_rating_result").Where("ImageName = ? AND SolutionID = ? AND UserName = ?", ratingResult.ImageName, solutionID, ratingResult.UserName).Count()
	if err != nil {
		return err
	}

	if count != 0 {
		newValues := map[string]interface{}{
			"ImageName":                   ratingResult.ImageName,
			"SolutionID":                  solutionID,
			"UserName":                    ratingResult.UserName,
			"RatingEnum":                  ratingResult.RatingEnum,
			"AdditionalRatingDescription": ratingResult.AdditionalRatingDescription,
			"UploadTime":                  ratingResult.UploadTime,
		}
		// 更新记录
		affected, err2 := session.Table("t_rating_result").Where("ImageName = ? AND SolutionID = ? AND UserName = ?", ratingResult.ImageName, solutionID, ratingResult.UserName).Update(newValues)
		if err2 != nil {
			err3 := session.Rollback()
			if err3 != nil {
				log.WithFields(log.Fields{
					"event": "Insert rating result",
					"err":   err3,
				}).Infof("Failed")
			}
			return err2
		} else {
			// 处理更新成功
			fmt.Printf("更新了 %d 条记录\n", affected)
		}
	} else {
		// 插入新的评分结果到数据库
		var tRatingResult = TRatingResult{
			ImageName:                   ratingResult.ImageName,
			SolutionID:                  solutionID,
			UserName:                    ratingResult.UserName,
			RatingEnum:                  ratingResult.RatingEnum,
			AdditionalRatingDescription: ratingResult.AdditionalRatingDescription,
			UploadTime:                  ratingResult.UploadTime,
		}
		_, err2 := session.Insert(&tRatingResult)
		if err2 != nil {
			err3 := session.Rollback()
			if err3 != nil {
				log.WithFields(log.Fields{
					"event": "Insert rating result",
					"err":   err3,
				}).Infof("Failed")
			}
			return fmt.Errorf("failed to insert rating result: %v", err)
		}

		// 检查UserImageMapCachedData
		usernames, ok := UserImageMapCachedData.Data[ratingResult.ImageName]
		if ok && len(usernames) > 2 {
			delete(UserImageMapCachedData.Data, ratingResult.ImageName)
		}
	}
	// 提交事务
	if err2 := session.Commit(); err2 != nil {
		log.WithFields(log.Fields{
			"event": "Get rating result",
			"err":   err2,
		}).Infof("Failed")
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

func QueryRatingResult(queryInfo QueryRatingResultInfo) ([]RatingResultV2, error) {
	solutionID2NameMap := make(map[int64]string)
	solutionName2IDMap := make(map[string]int64)

	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Get rating result",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "Get rating result",
			"err":   err,
		}).Infof("Failed")
	}
	var solutions []TRatingSolution
	err := session.Table("t_rating_solution").Find(&solutions)
	if err != nil {
		return nil, err
	}

	for _, solution := range solutions {
		solutionID2NameMap[solution.ID] = solution.SolutionName
		solutionName2IDMap[solution.SolutionName] = solution.ID
	}

	var tmpResults []TRatingResult
	err = session.Table("t_rating_result").
		Where("(SolutionID = ? OR ? = 'All')", solutionName2IDMap[queryInfo.SolutionName], queryInfo.SolutionName).
		Where("(UserName = ? OR ? = 'All')", queryInfo.UserName, queryInfo.UserName).
		Where("UploadTime > ? AND UploadTime < ?", queryInfo.StartTime, queryInfo.EndTime).
		Find(&tmpResults)
	if err != nil {
		return nil, err
	}

	// 提交事务
	if err2 := session.Commit(); err2 != nil {
		log.WithFields(log.Fields{
			"event": "Get rating result",
			"err":   err2,
		}).Infof("Failed")
	}

	var results []RatingResultV2
	for _, tmpResult := range tmpResults {
		results = append(results, RatingResultV2{
			ImageName:                   tmpResult.ImageName,
			SolutionName:                solutionID2NameMap[tmpResult.SolutionID],
			UserName:                    tmpResult.UserName,
			RatingEnum:                  tmpResult.RatingEnum,
			AdditionalRatingDescription: tmpResult.AdditionalRatingDescription,
			UploadTime:                  tmpResult.UploadTime,
		})
	}

	return results, nil
}

func GetRatingUserName(SolutionName string) ([]string, error) {
	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Get rating user name",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "Get rating user name",
			"err":   err,
		}).Infof("Failed")
	}

	var solutionID int64
	if SolutionName != "All" {
		has, err := session.Table("t_rating_solution").Cols("ID").Where("SolutionName = ?", SolutionName).Get(&solutionID)
		if err != nil {
			return nil, err
		}
		if !has {
			return nil, nil
		}
	}

	var usernameResult []string
	err := session.Table("t_rating_result").Cols("UserName").
		Where("SolutionID = ? OR ? = 'All'", solutionID, SolutionName).
		Distinct("UserName").
		Find(&usernameResult)
	if err != nil {
		return nil, err
	}

	// 提交事务
	if err2 := session.Commit(); err2 != nil {
		log.WithFields(log.Fields{
			"event": "Get rating user name",
			"err":   err2,
		}).Infof("Failed")
	}

	return usernameResult, nil
}

func UpdateRatingSolution(updatedData []UpdateRatingSolutionInfo) error {
	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Update rating solution",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "Update rating solution",
			"err":   err,
		}).Infof("Failed")
	}
	// 循环逐个更新
	for _, data := range updatedData {
		updates := make(map[string]interface{})
		if data.NewSolutionName != "" {
			updates["NewSolutionName"] = data.NewSolutionName
		}
		if data.SolutionDetail != "" {
			updates["SolutionDetail"] = data.SolutionDetail
		}

		// 只更新有实际值的字段
		if len(updates) > 0 {
			_, err := session.Table("t_rating_solution").Where("SolutionName = ?", data.OldSolutionName).Update(updates)
			if err != nil {
				err2 := session.Rollback()
				if err2 != nil {
					log.WithFields(log.Fields{
						"event": "Update rating solution",
						"err":   err2,
					}).Infof("Failed")
				}
				return err
			}
		}
	}

	// 提交事务
	if err := session.Commit(); err != nil {
		log.WithFields(log.Fields{
			"event": "Update rating solution",
			"err":   err,
		}).Infof("Failed")
	}

	return nil
}

func GetRatingSolution() ([]TRatingSolution, error) {
	var results []TRatingSolution
	session := utils.DB.Table("t_rating_solution").Where("IsDeleted = ?", 0)
	err := session.Find(&results)
	if err != nil {
		return nil, err
	}

	return results, nil
}

func DeleteRatingSolution(deletedData []string) error {
	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Delete rating solution",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "Delete rating solution",
			"err":   err,
		}).Infof("Failed")
	}
	// 循环逐个更新
	for _, data := range deletedData {
		if data == "" {
			continue
		}

		// 只更新有实际值的字段
		_, err := session.Table("t_rating_solution").Where("SolutionName = ?", data).Update(map[string]interface{}{
			"IsDeleted": 1,
		})
		if err != nil {
			err2 := session.Rollback()
			if err2 != nil {
				log.WithFields(log.Fields{
					"event": "Delete rating solution",
					"err":   err2,
				}).Infof("Failed")
			}
			return err
		}
	}

	// 提交事务
	if err := session.Commit(); err != nil {
		log.WithFields(log.Fields{
			"event": "Delete rating solution",
			"err":   err,
		}).Infof("Failed")
	}

	return nil
}

func AddRatingSolution(addedData []AddRatingSolutionInfo) error {
	// 创建一个新的 Session 对象
	session := utils.DB.NewSession()
	defer func(session *xorm.Session) {
		err := session.Close()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "Add rating solution",
				"err":   err,
			}).Infof("Failed")
		}
	}(session)
	// 开始事务
	if err := session.Begin(); err != nil {
		log.WithFields(log.Fields{
			"event": "Add rating solution",
			"err":   err,
		}).Infof("Failed")
	}
	// 循环逐个添加
	for _, data := range addedData {
		if data.SolutionName == "" || data.SolutionDetail == "" {
			continue
		}

		// 只添加有实际值的方案
		_, err := session.Table("t_rating_solution").Insert(
			TRatingSolution{
				SolutionName:   data.SolutionName,
				SolutionDetail: data.SolutionDetail,
				IsDeleted:      false,
			},
		)
		if err != nil {
			err2 := session.Rollback()
			if err2 != nil {
				log.WithFields(log.Fields{
					"event": "Add rating solution",
					"err":   err2,
				}).Infof("Failed")
			}
			return err
		}
	}

	// 提交事务
	if err := session.Commit(); err != nil {
		log.WithFields(log.Fields{
			"event": "Add rating solution",
			"err":   err,
		}).Infof("Failed")
	}

	return nil
}
