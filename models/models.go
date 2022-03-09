package models

import (
	"time"
)

type TAnotation struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Soma      string    `xorm:"not null index VARCHAR(100)"`
	Owner     string    `xorm:"not null index VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TArbor struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Somaid    string    `xorm:"not null index VARCHAR(100)"`
	Image     string    `xorm:"not null index VARCHAR(100)"`
	X         float64   `xorm:"not null DECIMAL(10,3)"`
	Y         float64   `xorm:"not null DECIMAL(10,3)"`
	Z         float64   `xorm:"not null DECIMAL(10)"`
	Status    int       `xorm:"not null default 0 index(t_arbor_Is_deleted_Status_index) INT"`
	Owner     string    `xorm:"not null default '' index VARCHAR(100)"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arbor_Is_deleted_Status_index) INT"`
}

type TArborresult struct {
	Id          int       `xorm:"not null pk autoincr INT"`
	X           float64   `xorm:"not null DECIMAL(10,3)"`
	Y           float64   `xorm:"not null DECIMAL(10,3)"`
	Z           float64   `xorm:"not null DECIMAL(10,3)"`
	Type        int       `xorm:"not null default 0 INT"`
	Owner       string    `xorm:"not null index VARCHAR(100)"`
	Arborname   string    `xorm:"not null index VARCHAR(100)"`
	Ctime       time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime       time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
	Isdeleted   int       `xorm:"not null default 0 INT"`
	Updateowner string    `xorm:"not null index VARCHAR(100)"`
}

type TEffectSoma struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	X         float64   `xorm:"not null DECIMAL(10,3)"`
	Y         float64   `xorm:"not null DECIMAL(10,3)"`
	Z         float64   `xorm:"not null DECIMAL(10,3)"`
	Imageid   string    `xorm:"not null index VARCHAR(100)"`
	From      int       `xorm:"not null comment('来源 0:来自t_somainfo') INT"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TImage struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Detail    string    `xorm:"not null JSON"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TPotentialsomalocation struct {
	Id        int       `xorm:"not null pk autoincr comment('序号') INT"`
	Image     string    `xorm:"not null comment('所在图像编号') index unique(t_potentialsomalocation_loc) VARCHAR(100)"`
	X         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Y         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Z         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Type      int       `xorm:"default 0 comment('位置点是否有效 -1:无效位置 0:有效 ') INT"`
	Dataset   int       `xorm:"default 0 comment('图像数据集') INT"`
	Owner     string    `xorm:"not null default '' comment('访问者的用户名') index VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 comment('是否有效，0为有效，非0无效') unique(t_potentialsomalocation_loc) INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('创建时间') TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('最后一次更新时间') TIMESTAMP updated"`
}

type TSomainfo struct {
	Id        int       `xorm:"not null pk autoincr comment('序号') INT"`
	Name      string    `xorm:"not null comment('soma在图像内的编号e.18454_00001') unique VARCHAR(100)"`
	Image     string    `xorm:"not null comment('关联的图像编号') index unique(t_somainfo_loc) VARCHAR(100)"`
	X         float64   `xorm:"not null unique(t_somainfo_loc) DECIMAL(10,3)"`
	Y         float64   `xorm:"not null unique(t_somainfo_loc) DECIMAL(10,3)"`
	Z         float64   `xorm:"not null unique(t_somainfo_loc) DECIMAL(10,3)"`
	Location  int       `xorm:"not null comment('关联的潜在位置编号') index INT"`
	Client    int       `xorm:"default 0 comment('创建设备 0:Hi5') INT"`
	Owner     string    `xorm:"not null comment('创建者用户名') index VARCHAR(100)"`
	Updater   string    `xorm:"not null default '' comment('修改用户') VARCHAR(100)"`
	Color     string    `xorm:"not null default '0000ff' comment('颜色，默认为蓝色') VARCHAR(100)"`
	Status    int       `xorm:"not null default 0 comment('状态 0:未检查 1:检查，正确 2:检查，错误') index INT"`
	Methods   string    `xorm:"comment('refine方法和参数') JSON"`
	Refinedid int       `xorm:"not null default 0 comment('refine结果的id') INT"`
	Isdeleted int       `xorm:"not null default 0 comment('是否有效，0为有效，非0无效') unique(t_somainfo_loc) INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('创建时间') TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('更新时间') TIMESTAMP updated"`
}

type TUserinfo struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null comment('用户名') unique VARCHAR(100)"`
	Email     string    `xorm:"not null comment('邮箱') unique VARCHAR(100)"`
	Nickname  string    `xorm:"not null comment('昵称') VARCHAR(100)"`
	Passwd    string    `xorm:"not null comment('密码') VARCHAR(100)"`
	Score     int       `xorm:"not null default 0 comment('得分') index INT"`
	Appkey    string    `xorm:"not null default '' comment('网易云信appkey') VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}
