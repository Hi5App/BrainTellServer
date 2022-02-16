package services

import (
	log "github.com/sirupsen/logrus"
	"os/exec"
	"testing"
)

func TestLog(t *testing.T) {
	//log.Infoln(fmt.Sprintf("%s %s %s %s %s %s &", utils.CollaborateBinPath, port, utils.MainPath, p.Image, p.Neuron, p.Ano))
	//cmd := exec.Command("/bin/sh", "-c", fmt.Sprintf("%s %s %s %s %s %s &", utils.CollaborateBinPath, port, utils.MainPath, p.Image, p.Neuron, p.Ano))
	if true {
		cmd := exec.Command("/bin/sh", "-c", "ping 127.0.0.1 &")
		if err := cmd.Start(); err != nil {
			log.Error(err.Error())
		}
		if err := cmd.Wait(); err != nil {
			log.Error(err.Error())
		}
	}

	//log.Infoln("start process " + p.Ano + " success")
	for {

	}
}
