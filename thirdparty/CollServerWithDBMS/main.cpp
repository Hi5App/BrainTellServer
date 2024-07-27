#include <QCoreApplication>
#include "coll_server.h"
#include <stdio.h>
#include "utils.h"
#include <signal.h>
#include <unistd.h>
QFile *logfile=nullptr;

void sighandle(int sig){
    logfile->flush();
    //fsync的功能是确保文件fd所有已修改的内容已经正确同步到硬盘上，该调用会阻塞等待直到设备报告IO完成。
    fsync(1);fsync(2);
    exit(0);
}

int main(int argc, char *argv[])
{
    //控制台应用程序
    QCoreApplication a(argc, argv);
    //发生中断时，进入sighandle函数，将缓存区里的数据写入磁盘
    signal(SIGINT,sighandle);
    if(argc<8) return -1;

    QString port=argv[1];
    QString prefix=argv[2];
    QString image=argv[3];
    QString neuron=argv[4];
    QString anoname=argv[5];
    QString maxUserNums=argv[6];
    QString modelDetectIntervals=argv[7];

    //将标准输出和标准错误流中的内容追加到日志文件中
    freopen((prefix+"/log/"+anoname+".txt").toStdString().c_str(),"a",stdout);
    freopen((prefix+"/log/"+anoname+".txt").toStdString().c_str(),"a",stderr);

    int maxUserNumsInt = maxUserNums.toInt();
    int modelDetectIntervalsInt = modelDetectIntervals.toInt();
    auto server=new CollServer(port,image,neuron,anoname,prefix,maxUserNumsInt,modelDetectIntervalsInt);
    logfile=new QFile(prefix+"/orders/"+anoname+".txt",server);
    logfile->open(QIODevice::Append);
    return a.exec();

}
