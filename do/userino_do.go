package do

import (
	"BrainTellServer/models"
	"BrainTellServer/utils"
	"context"
	"errors"
	"github.com/google/uuid"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"time"
)

type UserInfo = utils.UserInfo
type GameUserInfo = utils.GameUserInfo

//func QueryUser(pa *models.TUserinfo, pd *utils.QueryCondition) ([]*UserInfo, error) {
//	jsonpa, _ := jsoniter.MarshalToString(pa)
//
//	users := make([]*models.TUserinfo, 0)
//	session := utils.DB.NewSession()
//	defer session.Close()
//	session.Where("Isdeleted = ?", 0)
//	if pd != nil {
//		session = session.Limit(pd.Limit, pd.Off)
//	}
//	//可以直接用pa作为参数吗
//	//当使用结构作为条件查询时,XORM只会查询非零值字段。
//	err := session.Find(&users, &models.TUserinfo{
//
//		Name:   pa.Name,
//		Email:  pa.Email,
//		Passwd: pa.Passwd,
//	})
//
//	fmt.Printf("----------userinfo_do QueryUser Find error: %v-------------------\n", err)
//
//	if err != nil {
//		log.WithFields(log.Fields{
//			"event": "Query userinfo",
//			"pa":    jsonpa,
//		}).Warnf("%v\n", err)
//		return nil, err
//	}
//
//	res := make([]*UserInfo, 0)
//	for _, user := range users {
//		res = append(res, &UserInfo{
//			Id:       user.Id,
//			Name:     user.Name,
//			Email:    user.Email,
//			Score:    user.Score,
//			AppKey:   user.Appkey,
//			NickName: user.Nickname,
//		})
//	}
//
//	fmt.Printf("----------userinfo_do QueryUser Find result: %v-------------------\n", res)
//
//	jsonres, _ := jsoniter.MarshalToString(res)
//	log.WithFields(log.Fields{
//		"event": "Query userinfo",
//		"pa":    jsonpa,
//		"RES":   jsonres,
//	}).Infof("Success")
//
//	return res, nil
//}

func QueryUser(userMetaInfo *models.UserMetaInfoV1, userName string) ReturnWrapper {
	var userCollection = utils.UserDB

	result := userCollection.FindOne(
		context.TODO(),
		bson.D{{"Name", userName}})

	if result.Err() != nil {
		return ReturnWrapper{false, "Cannot find target user!"}
	} else {
		err := result.Decode(userMetaInfo)
		if err != nil {
			return ReturnWrapper{false, err.Error()}
		} else {
			return ReturnWrapper{true, ""}
		}
	}
}

//func InsertUser(pa *models.TUserinfo) (int64, error) {
//	jsonpa, _ := jsoniter.MarshalToString(pa)
//	session := utils.DB.NewSession()
//	defer session.Close()
//	affect, err := session.Insert(pa)
//	if err != nil {
//		log.WithFields(log.Fields{
//			"event": "Insert userinfo",
//			"pa":    jsonpa,
//		}).Warnf("%v\n", err)
//		return 0, err
//	}
//
//	log.WithFields(log.Fields{
//		"event":  "Insert userinfo",
//		"pa":     jsonpa,
//		"affect": affect,
//	}).Infof("Success")
//	return affect, nil
//}

type ReturnWrapper struct {
	Status  bool
	Message string
}

func GetNewUserIdAndIncrease() (ReturnWrapper, int32) {
	collection := utils.UserDB

	var result struct {
		Seq int32
	}

	val := "CurrentNewUserId"
	filter := bson.D{{"AttributeName", val}}
	update := bson.D{
		{"$inc", bson.D{{"seq", 1}}},
		{"$setOnInsert", bson.D{{"AttributeName", val}}},
	}
	opts := options.FindOneAndUpdate().SetUpsert(true).SetReturnDocument(options.After)

	err := collection.FindOneAndUpdate(context.Background(), filter, update, opts).Decode(&result)
	if err != nil {
		return ReturnWrapper{
			Status:  false,
			Message: err.Error(),
		}, -1
	}

	return ReturnWrapper{
		Status:  true,
		Message: "GetNewUserId Successfully!",
	}, result.Seq
}

func InsertUser(userInfo *models.UserMetaInfoV1) ReturnWrapper {

	status, newUserId := GetNewUserIdAndIncrease()
	if !status.Status {
		return status
	}

	userMetaInfo := models.UserMetaInfoV1{
		Base: models.MetaInfoBase{
			Id:                     primitive.NewObjectID(),
			DataAccessModelVersion: "V1",
			Uuid:                   uuid.NewString(),
		},
		Name:                userInfo.Name,
		Password:            userInfo.Password,
		Description:         "BrainTell Server User Account.",
		CreateTime:          time.Now(),
		HeadPhotoBinData:    nil,
		UserPermissionGroup: "Default",
		UserId:              newUserId,
		CompatibleData: models.BrainTellServerMysqlDBCompatibleData{
			Email:     userInfo.CompatibleData.Email,
			NickName:  userInfo.CompatibleData.NickName,
			Score:     userInfo.CompatibleData.Score,
			Appkey:    userInfo.CompatibleData.Appkey,
			Isdeleted: 0,
		},
	}

	var userCollection = utils.UserDB
	result := userCollection.FindOne(context.TODO(), bson.D{
		{"Name", userInfo.Name},
	})

	if result.Err() != nil {
		if errors.Is(result.Err(), mongo.ErrNoDocuments) {
			_, err := userCollection.InsertOne(context.TODO(), userMetaInfo)
			if err != nil {
				return ReturnWrapper{false, "Create user failed! Error:" + err.Error()}
			}
			return ReturnWrapper{true, "Create user successfully!"}
		}
		return ReturnWrapper{false, "Unknown error!"}
	} else {
		// find one means already exist
		return ReturnWrapper{false, "User already exist!"}
	}
}

//func UpdateUser(pa *models.TUserinfo, pc *models.TUserinfo) (int64, error) {
//	jsonpa, _ := jsoniter.MarshalToString(pa)
//
//	affect, err := utils.DB.NewSession().Update(pa, pc)
//	if err != nil {
//		log.WithFields(log.Fields{
//			"event": "Update userinfo",
//			"pa":    jsonpa,
//		}).Warnf("%v\n", err)
//		return 0, err
//	}
//
//	log.WithFields(log.Fields{
//		"event":  "Update userinfo",
//		"pa":     jsonpa,
//		"affect": affect,
//	}).Infof("Success")
//	return affect, nil
//}

//func DeltelUser(pa *models.TUserinfo) (int64, error) {
//	jsonpa, _ := jsoniter.MarshalToString(pa)
//
//	var pc *models.TUserinfo
//	*pc = *pa
//	pa.Isdeleted = 1
//	affect, err := utils.DB.NewSession().Update(pa, pc)
//	if err != nil {
//		log.WithFields(log.Fields{
//			"event": "Delete userinfo",
//			"pa":    jsonpa,
//		}).Warnf("%v\n", err)
//		return 0, err
//	}
//
//	log.WithFields(log.Fields{
//		"event":  "Delete userinfo",
//		"pa":     jsonpa,
//		"affect": affect,
//	}).Infof("Success")
//	return affect, nil
//}

//// QueryGameUser game func
//func QueryGameUser(pa *models.TGameUserinfo, pd *utils.QueryCondition) ([]*GameUserInfo, error) {
//	jsonpa, _ := jsoniter.MarshalToString(pa)
//
//	users := make([]*models.TGameUserinfo, 0)
//
//	//事务？
//	session := utils.DB.NewSession()
//	defer session.Close()
//	session.Where("Isdeleted = ?", 0)
//	if pd != nil {
//		session = session.Limit(pd.Limit, pd.Off)
//	}
//	err := session.Find(&users, &models.TGameUserinfo{
//		Name:   pa.Name,
//		Email:  pa.Email,
//		Passwd: pa.Passwd,
//	})
//
//	fmt.Printf("----------userinfo_do QueryUser Find error: %v-------------------\n", err)
//
//	if err != nil {
//		log.WithFields(log.Fields{
//			"event": "Query game userinfo",
//			"pa":    jsonpa,
//		}).Warnf("%v\n", err)
//		return nil, err
//	}
//
//	res := make([]*GameUserInfo, 0)
//	for _, user := range users {
//		res = append(res, &GameUserInfo{
//			Id:    user.Id,
//			Name:  user.Name,
//			Email: user.Email,
//			Score: user.Score,
//		})
//	}
//
//	fmt.Printf("----------userinfo_do QueryUser Find result: %v-------------------\n", res)
//
//	jsonres, _ := jsoniter.MarshalToString(res)
//	log.WithFields(log.Fields{
//		"event": "Query game userinfo",
//		"pa":    jsonpa,
//		"RES":   jsonres,
//	}).Infof("Success")
//
//	return res, nil
//}

// QueryGameUser game func
func QueryGameUser(userMetaInfo *models.UserMetaInfoV1, userName string) ReturnWrapper {
	var userCollection = utils.UserDB

	result := userCollection.FindOne(
		context.TODO(),
		bson.D{{"Name", userName}})

	if result.Err() != nil {
		return ReturnWrapper{false, "Cannot find target user!"}
	} else {
		err := result.Decode(userMetaInfo)
		if err != nil {
			return ReturnWrapper{false, err.Error()}
		} else {
			return ReturnWrapper{true, ""}
		}
	}
}
