package services

import (
	"BrainTellServer/ao"
	"BrainTellServer/utils"
	"github.com/robfig/cron"
	log "github.com/sirupsen/logrus"
	"gopkg.in/jordan-wright/email.v1"
	"net/smtp"
)

func SendPerformance() {
	c := cron.New()
	spec := "0 */1 * * * ?"
	c.AddFunc(spec, func() {
		log.Infof("cron")
		performance, dailyperformance, err := ao.GetUserPerformance()
		if err != nil {
			log.WithFields(log.Fields{
				"event": "SendPerformance",
			}).Error("GetUserPerformance failed")
		}

		html := "<html>\n<body>"
		html += utils.ConvertPerformance2html("Total Soma", performance)
		html += utils.ConvertPerformance2html("Daily Soma", dailyperformance)
		html += "</body>\n</html>"
		log.Infoln(html)
		e := email.NewEmail()
		e.From = "huhudexiaozhuzhu@126.com"
		e.To = []string{"1054067071@qq.com"}
		//e.To = []string{, "h@braintell.org"}
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
