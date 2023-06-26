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
	Somaid    string    `xorm:"not null unique(t_arbor_SomaId_X_Y_Z_uindex) VARCHAR(100)"`
	Image     string    `xorm:"not null index VARCHAR(100)"`
	X         float64   `xorm:"not null unique(t_arbor_SomaId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Y         float64   `xorm:"not null unique(t_arbor_SomaId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Z         float64   `xorm:"not null unique(t_arbor_SomaId_X_Y_Z_uindex) DECIMAL(10)"`
	Status    int       `xorm:"not null default 0 index(t_arbor_Is_deleted_Status_index) INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arbor_Is_deleted_Status_index) INT"`
}

type TArbordetail struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Arborid   int       `xorm:"not null index(t_arbordetail_ArborId_Isdeleted_index) unique(t_arbordetail_ArborId_X_Y_Z_Type_uindex) INT"`
	X         float64   `xorm:"not null unique(t_arbordetail_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Y         float64   `xorm:"not null unique(t_arbordetail_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Z         float64   `xorm:"not null unique(t_arbordetail_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Type      int       `xorm:"not null unique(t_arbordetail_ArborId_X_Y_Z_Type_uindex) INT"`
	Owner     string    `xorm:"not null index VARCHAR(200)"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arbordetail_ArborId_Isdeleted_index) INT"`
}

type TArborresult struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Arborid   int       `xorm:"not null index(t_arborresult_ArborId_Iddeleted_index) INT"`
	Result    int       `xorm:"not null comment('用户判断的结果') INT"`
	Form      int       `xorm:"not null comment('用户从什么方式获得数据') index INT"`
	Owner     string    `xorm:"not null index VARCHAR(200)"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arborresult_ArborId_Iddeleted_index) INT"`
}

type TEffectSoma struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	X         float64   `xorm:"not null unique(t_effect_soma_ImageId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Y         float64   `xorm:"not null unique(t_effect_soma_ImageId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Z         float64   `xorm:"not null unique(t_effect_soma_ImageId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Image     string    `xorm:"not null unique(t_effect_soma_ImageId_X_Y_Z_uindex) index VARCHAR(100)"`
	From      int       `xorm:"not null default 0 comment('来源 0:来自t_somainfo') INT"`
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
	Type      int       `xorm:"default 0 comment('位置点是否有效 -1:无效位置 0:未决定 1:有soma 2:无soma') INT"`
	Dataset   int       `xorm:"default 0 comment('图像数据集 0:soma') INT"`
	Owner     string    `xorm:"not null default '' comment('访问者的用户名') index VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 comment('是否有效，0为有效，非0无效') unique(t_potentialsomalocation_loc) INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('创建时间') TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('最后一次更新时间') TIMESTAMP updated"`
}

// t_somainfo_loc是联合唯一索引,
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
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Email     string    `xorm:"not null unique VARCHAR(100)"`
	Nickname  string    `xorm:"not null VARCHAR(100)"`
	Passwd    string    `xorm:"not null VARCHAR(100)"`
	Score     int       `xorm:"not null default 0 index INT"`
	Appkey    string    `xorm:"not null default '' comment('网易云信appkey') VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TGameUserinfo struct {
	Id        int       `xorm:"not null pk autoincr unique INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Email     string    `xorm:"not null unique VARCHAR(100)"`
	Passwd    string    `xorm:"not null VARCHAR(100)"`
	Score     int       `xorm:"not null default 0 comment('玩家练习模式最高分') INT"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' comment('创建时间') TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' comment('更新时间') TIMESTAMP updated"`
}

// correctbp, wrongbp, missedbp为什么是string类型
type TGameRecord struct {
	Id        int       `xorm:"not null pk autoincr unique INT"`
	Username  string    `xorm:"not null comment('玩家标识符') unique VARCHAR(100)"`
	Swcid     string    `xorm:"not null comment('swc标识符') VARCHAR(100)"`
	Correctbp string    `xorm:"not null default '' comment('正确的branching points的index') VARCHAR(100)"`
	Wrongbp   string    `xorm:"not null default '' comment('错误的Branching Point的Index') VARCHAR(100)"`
	Missedbp  string    `xorm:"not null default '' comment('缺失的，但应该是正确的Branching Point的index') VARCHAR(100)"`
	Points    int       `xorm:"not null default 0 comment('玩家该局得分') INT"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('创建时间') TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP comment('更新时间') TIMESTAMP updated"`
}

// bouton 检查相关的数据结构
type TArborBouton struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Somaid    string    `xorm:"not null unique(t_arbor_bouton_SomaId_X_Y_Z_uindex) VARCHAR(100)"`
	Image     string    `xorm:"not null index VARCHAR(100)"`
	X         string    `xorm:"not null unique(t_arbor_bouton_SomaId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Y         string    `xorm:"not null unique(t_arbor_bouton_SomaId_X_Y_Z_uindex) DECIMAL(10,3)"`
	Z         string    `xorm:"not null unique(t_arbor_bouton_SomaId_X_Y_Z_uindex) DECIMAL(10)"`
	Status    int       `xorm:"not null default 0 index(t_arbor_bouton_Is_deleted_Status_index) INT"`
	Ctime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arbor_bouton_Is_deleted_Status_index) INT"`
}

type TArbordetailBouton struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Arborid   int       `xorm:"not null index(t_arbordetail_bouton_ArborId_Isdeleted_index) unique(t_arbordetail_bouton_ArborId_X_Y_Z_Type_uindex) INT"`
	X         float64   `xorm:"not null unique(t_arbordetail_bouton_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Y         float64   `xorm:"not null unique(t_arbordetail_bouton_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Z         float64   `xorm:"not null unique(t_arbordetail_bouton_ArborId_X_Y_Z_Type_uindex) DECIMAL(10,3)"`
	Type      int       `xorm:"not null unique(t_arbordetail_bouton_ArborId_X_Y_Z_Type_uindex) INT"`
	Owner     string    `xorm:"not null index VARCHAR(200)"`
	Ctime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP updated"`
	Isdeleted int       `xorm:"not null default 0 index(t_arbordetail_bouton_ArborId_Isdeleted_index) INT"`
}

type TArborresultBouton struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Arborid   int       `xorm:"not null index(t_arborresult_bouton_ArborId_Iddeleted_index) unique(t_arborresult_bouton_ArborId_Owner_uindex) INT"`
	Result    int       `xorm:"not null comment('用户判断的结果') INT"`
	Form      int       `xorm:"not null comment('用户从什么方式获得数据') index INT"`
	Owner     string    `xorm:"not null unique(t_arborresult_bouton_ArborId_Owner_uindex) index VARCHAR(200)"`
	Isdeleted int       `xorm:"not null default 0 index(t_arborresult_bouton_ArborId_Iddeleted_index) INT"`
	Ctime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default 'CURRENT_TIMESTAMP' TIMESTAMP updated"`
}
