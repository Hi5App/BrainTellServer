package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	"github.com/robfig/cron"
	log "github.com/sirupsen/logrus"
	"gopkg.in/jordan-wright/email.v1"
	"net/smtp"
)

// SendPerformance 定期给老师发送邮件
func SendPerformance() {
	c := cron.New()
	spec := "0 0 23 * * ?"
	c.AddFunc(spec, func() {
		log.Infof("cron")
		totalsoma, dailysoma, err := ao.GetSomaCnt()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "SendPerformance",
			}).Error("get soma failed")
		}

		totalarbor,dailtarbor,err:=ao.GetCheckCnt()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "SendPerformance",
			}).Error("get arbor failed")
		}
		html := "<html>\n<body>"
		html += utils.ConvertPerformance2html("Total Proof", totalarbor)
		html += utils.ConvertPerformance2html("Daily Proof", dailtarbor)
		html += utils.ConvertPerformance2html("Total Soma", totalsoma)
		html += utils.ConvertPerformance2html("Daily Soma", dailysoma)
		html += "</body>\n</html>"
		e := email.NewEmail()
		e.From = "huhudexiaozhuzhu@126.com"
		e.To = utils.Emails
		e.Bcc = []string{"1054067071@qq.com"}
		e.Subject = "BrainTell Soma Report"
		e.HTML = []byte(html)
		err = e.Send("smtp.126.com:25",
			smtp.PlainAuth("",
				"huhudexiaozhuzhu@126.com",
				"NESBNCIOAGXZMJIN",
				"smtp.126.com"))

		if err != nil {
			log.Errorln(err)
			return
		}
	})
	c.Start()
}
