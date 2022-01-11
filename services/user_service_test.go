package services

import (
	log "github.com/sirupsen/logrus"
	"testing"
)

func TestLog(t *testing.T) {
	log.WithFields(log.Fields{
		"animal": "walrus",
	}).Info("A walrus appears")
}
