package do

import (
	"BrainTellServer/models"
	"github.com/sirupsen/logrus"
	"testing"
	"xorm.io/xorm"
)

func TestUserLogin(t *testing.T) {
	db, _ := xorm.NewEngine("mysql", "root@1234:tcp(127.0.0.1:3306/Brain")
	db.Where("Isdeleted = ?", 0)
	users := make([]*models.TUserinfo, 0)
	db.Find(users)

	logrus.Infoln("s")
}
