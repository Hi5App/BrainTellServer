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

type TImage struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Detail    string    `xorm:"not null JSON"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TPotentialsomalocation struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Image     string    `xorm:"not null index VARCHAR(100)"`
	X         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Y         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Z         int       `xorm:"not null unique(t_potentialsomalocation_loc) INT"`
	Owner     string    `xorm:"not null default '' index VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TSomainfo struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null comment('e.18454_00001') unique VARCHAR(100)"`
	Image     string    `xorm:"not null index VARCHAR(100)"`
	X         float64   `xorm:"not null unique(t_somainfo_loc) INT"`
	Y         float64   `xorm:"not null unique(t_somainfo_loc) INT"`
	Z         float64   `xorm:"not null unique(t_somainfo_loc) INT"`
	Location  int       `xorm:"not null index INT"`
	Owner     string    `xorm:"not null index VARCHAR(100)"`
	Color     string    `xorm:"not null default '0000ff' VARCHAR(100)"`
	Status    int       `xorm:"not null default 0 comment('o:新插入') index INT"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}

type TUserinfo struct {
	Id        int       `xorm:"not null pk autoincr INT"`
	Name      string    `xorm:"not null unique VARCHAR(100)"`
	Email     string    `xorm:"not null unique VARCHAR(100)"`
	Nickname  string    `xorm:"not null unique VARCHAR(100)"`
	Passwd    string    `xorm:"not null VARCHAR(100)"`
	Score     int       `xorm:"not null default 0 index INT"`
	Appkey    string    `xorm:"not null default '' comment('网易云信appkey') VARCHAR(100)"`
	Isdeleted int       `xorm:"not null default 0 INT"`
	Ctime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP created"`
	Mtime     time.Time `xorm:"not null default CURRENT_TIMESTAMP TIMESTAMP updated"`
}
