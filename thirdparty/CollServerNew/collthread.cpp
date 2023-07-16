#include "collthread.h"
#include "coll_server.h"

CollThread::CollThread(QObject* parent):myServer(static_cast<CollServer*>(parent))
{

}

CollThread::~CollThread(){

}

void CollThread::setServer(CollServer* curServer){
    myServer=curServer;
}

void CollThread::run(){
    qDebug()<<"7";
//    sockethelper->setServer(myServer);
    exec();
}

