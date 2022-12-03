#include <QCoreApplication>
#include "coll_server.h"
#include <stdio.h>
#include "utils.h"
#include <signal.h>
#include <unistd.h>
QFile *logfile=nullptr;

void sighandle(int sig){
    logfile->flush();
    fsync(1);fsync(2);
    exit(0);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    signal(SIGINT,sighandle);
    if(argc<6) return -1;

    QString port=argv[1];
    QString prefix=argv[2];
    QString image=argv[3];
    QString neuron=argv[4];
    QString anoname=argv[5];

    freopen((prefix+"/log/"+anoname+".txt").toStdString().c_str(),"a",stdout);
    freopen((prefix+"/log/"+anoname+".txt").toStdString().c_str(),"a",stderr);

    auto server=new CollServer(port,image,neuron,anoname,prefix);
    logfile=new QFile(prefix+"/orders/"+anoname+".txt",server);
    logfile->open(QIODevice::Append);
    return a.exec();

}
