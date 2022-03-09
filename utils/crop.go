package utils

/*
#cgo CFLAGS: -I../cpp
#cgo LDFLAGS: -L./ -lcropswc
#include "cwrap.h"
*/
import "C"
import (
	"context"
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
	"os"
	"os/exec"
	"time"
)

type BBox struct {
	Pa1 XYZ    `json:"pa1"`
	Pa2 XYZ    `json:"pa2"`
	Res string `json:"res"`
	Obj string `json:"obj"`
}

func GetBBImage(pa *BBox) (string, error) {
	ctx := context.TODO()
	if err := availableCropProcess.Acquire(ctx, 1); err != nil {
		log.Infof("Failed to acquire semaphore: %v\n", err)
		return "", errors.New(fmt.Sprintf("Failed to acquire semaphore: %v", err))
	}
	defer availableCropProcess.Release(1)
	savefile := Tmpdir + "/" + fmt.Sprintf("%s_%d_%d_%d_%d_%d_%d_%d.v3dpbd", pa.Obj,
		int(pa.Pa1.X), int(pa.Pa1.Y), int(pa.Pa1.Z),
		int(pa.Pa2.X), int(pa.Pa2.Y), int(pa.Pa2.Z),
		time.Now().UnixNano())
	cmd := exec.Command(Vaa3dBin, ImageDir+"/"+pa.Obj+"/"+pa.Res, savefile,
		fmt.Sprint(int(pa.Pa1.X)), fmt.Sprint(int(pa.Pa1.Y)), fmt.Sprint(int(pa.Pa1.Z)),
		fmt.Sprint(int(pa.Pa2.X)), fmt.Sprint(int(pa.Pa2.Y)), fmt.Sprint(int(pa.Pa2.Z)),
	)
	log.Infoln(cmd.String())
	cmd.Env = append(os.Environ())

	out, err := cmd.Output()

	if err != nil {
		log.WithFields(
			log.Fields{
				"event":  "crop image",
				"status": "Failed",
				"out":    string(out),
			}).Warnf("%s\n", err)
		return "", err
	}
	log.WithFields(
		log.Fields{
			"event":  "crop image",
			"status": "Success",
			"out":    string(out),
		}).Infof("\n")
	return savefile, nil
}

func GetBBSwc(pa *BBox) (string, error) {
	savefile := Tmpdir + "/" + fmt.Sprintf("%s_%d_%d_%d_%d_%d_%d_%d.eswc", pa.Obj,
		int(pa.Pa1.X), int(pa.Pa1.Y), int(pa.Pa1.Z),
		int(pa.Pa2.X), int(pa.Pa2.Y), int(pa.Pa2.Z),
		time.Now().UnixNano())
	res := C.getSwcInBlock(C.CString(DataPath+pa.Obj), C.int(pa.Pa1.X), C.int(pa.Pa2.X), C.int(pa.Pa1.Y), C.int(pa.Pa2.Y), C.int(pa.Pa1.Z), C.int(pa.Pa2.Z), C.CString(savefile))
	if res == 0 {
		return "", errors.New("crop Swc Failed")
	}
	return savefile, nil
}
