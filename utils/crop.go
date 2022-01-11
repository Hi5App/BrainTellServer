package utils

import (
	"context"
	"errors"
	"fmt"
	log "github.com/sirupsen/logrus"
	"golang.org/x/sync/semaphore"
	"os"
	"os/exec"
	"time"
)

var availableCropProcess = semaphore.NewWeighted(20)

func GetBB(pa *CropBB) (string, error) {
	ctx := context.TODO()
	if err := availableCropProcess.Acquire(ctx, 1); err != nil {
		log.Infof("Failed to acquire semaphore: %v\n", err)
		return "", errors.New(fmt.Sprintf("Failed to acquire semaphore: %v", err))
	}
	defer availableCropProcess.Release(1)
	savefile := Tmpdir + "/" + pa.User.Name + "_X" + fmt.Sprint(pa.Loc.X) + "_Y" + fmt.Sprint(pa.Loc.Y) + "_Z" + fmt.Sprint(pa.Loc.Z) + "_L" + fmt.Sprint(pa.Len) + fmt.Sprint(time.Now().UnixNano()) + ".v3dpbd"
	cmd := exec.Command(Vaa3dBin, ImageDir+"/"+pa.Image+"/"+pa.RES, savefile, fmt.Sprint(pa.Loc.X), fmt.Sprint(pa.Loc.Y), fmt.Sprint(pa.Loc.Z), fmt.Sprint(pa.Len))
	cmd.Env = append(os.Environ())

	out, err := cmd.Output()
	log.WithFields(
		log.Fields{
			"event":  "crop image",
			"status": "Failed",
			"out":    string(out),
		}).Warnf("%s\n", err)
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
