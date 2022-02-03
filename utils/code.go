package utils

import (
	"crypto/aes"
	"crypto/cipher"
	"fmt"
	log "github.com/sirupsen/logrus"
)

var commonIV = []byte{0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0e, 0x0f}

// Encode 加密
func Encode(raw []byte) ([]byte, error) {
	c, err := aes.NewCipher([]byte(AesKey))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Encode userinfo",
		}).Warnf("%v\n", err)
		return raw, err
	}

	cfb := cipher.NewCFBEncrypter(c, commonIV)
	ciphertext := make([]byte, len(raw))
	cfb.XORKeyStream(ciphertext, raw)
	return ciphertext, nil
}

// Decode 解密
func Decode(coded []byte) ([]byte, error) {
	c, err := aes.NewCipher([]byte(AesKey))
	if err != nil {
		log.WithFields(log.Fields{
			"event": "Encode userinfo",
		}).Warnf("%v\n", err)
		return coded, err
	}
	cfbdec := cipher.NewCFBDecrypter(c, commonIV)
	plaintextCopy := make([]byte, 0)
	cfbdec.XORKeyStream(plaintextCopy, coded)
	return plaintextCopy, nil
}

// ConvertPerformance2html 将点soma的信息编码为html
func ConvertPerformance2html(tablename string, performance map[string]int64) string {
	var sum int64 = 0
	var performaccehtml string
	performaccehtml = fmt.Sprintf("<h4>%s</h4>\n", tablename)
	performaccehtml += fmt.Sprintf("<table>\n")
	for key, value := range performance {
		performaccehtml += fmt.Sprintf("<tr>\n<td>%s</td>\n<td>%d</td></tr>\n", key, value)
		sum += value
	}
	performaccehtml += fmt.Sprintf("<tr>\n<td>%s</td>\n<td>%d</td></tr>\n", "total", sum)
	performaccehtml += fmt.Sprintf("</table>\n")
	return performaccehtml
}
