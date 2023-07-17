#include "coll_server.h"
#include <unistd.h>
#include "utils.h"
#include <thread>
#include <QThread>
#include <chrono>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <algorithm>
extern QFile *logfile;

CollServer* CollServer::curServer=nullptr;

QStringList CollServer::msglist;
int CollServer::processedmsgcnt=0;
int CollServer::savedmsgcnt=0;
int CollServer::receivedcnt=0;
QMap<QString,CollClient*> CollServer::hashmap;
V_NeuronSWC_list CollServer::segments;
QList<CellAPO> CollServer::markers;
//int receivedcnt;
QString CollServer::swcpath;
QString CollServer::apopath;
QString CollServer::anopath;
QMutex CollServer::mutex;
QString CollServer::RES;

CollServer::CollServer(QString port,QString image,QString neuron,QString anoname,QString prefix,QObject *parent)
    :QTcpServer(parent),Port(port),Image(image),Neuron(neuron),AnoName(anoname),Prefix(prefix+"/data/"+image+"/"+neuron+"/"+anoname)
    ,timerForAutoSave(new QTimer(this)),timerForDetectOthers(new QTimer(this)),timerForDetectTip(new QTimer(this)),
    timerForDetectCrossing(new QTimer(this)),timerForAutoExit(new QTimer(this))
{
    qDebug()<<"MainThread:"<<QThread::currentThreadId();
    curServer=this;
    detectUtil=new CollDetection(this,this);
    detectUtil->getSomaCoordinate(anopath);

    qRegisterMetaType<qintptr>("qintptr");

    auto nt=readSWC_file(Prefix+"/"+AnoName+".ano.eswc");
    segments=NeuronTree__2__V_NeuronSWC_list(nt);
    markers=readAPO_file(Prefix+"/"+AnoName+".ano.apo");
    swcpath=Prefix+"/"+AnoName+".ano.eswc";
    apopath=Prefix+"/"+AnoName+".ano.apo";
    anopath=Prefix+"/"+AnoName+".ano";

    // 3分钟执行一次
    timerForAutoSave->start(3*60*1000);
    timerForDetectTip->start(24*60*60*1000);
    timerForDetectCrossing->start(24*60*60*1000);
    timerForDetectOthers->start(60*1000);
    timerForAutoExit->start(24*60*60*1000);
    CollClient::timerforupdatemsg.start(1000);
    // 为msglist这个列表分配内存
    msglist.reserve(5000);

    // If address is QHostAddress::Any, the server will listen on all network interfaces.
    if(!this->listen(QHostAddress::Any,Port.toInt())){
        std::cerr<<"Can not init server with port "<<Port.toInt()<<std::endl;
        setexpire(Port.toInt(),AnoName.toStdString().c_str(),5);
        recoverPort(Port.toInt());
        std::cerr<<AnoName.toStdString()+" server is released\n";
        exit(-1);
    }
//    QTimer::singleShot(24*60*60*1000,this,[this]{
//        setexpire(Port.toInt(),AnoName.toStdString().c_str(),5);
//        recoverPort(Port.toInt());
//        std::cerr<<AnoName.toStdString()+" server is released\n";
//        logfile->flush();
//        exit(0);
//    });
    connect(timerForDetectOthers,&QTimer::timeout,detectUtil,&CollDetection::detectOthers);
    connect(timerForDetectTip,&QTimer::timeout,detectUtil,&CollDetection::detectTips);
    connect(timerForDetectCrossing,&QTimer::timeout,detectUtil,&CollDetection::detectCrossings);
    connect(timerForAutoSave,&QTimer::timeout,this,&CollServer::autoSave);
    connect(timerForAutoExit,&QTimer::timeout,this,&CollServer::autoExit);
    connect(&CollClient::timerforupdatemsg,&QTimer::timeout,[]{
        for (auto iter=hashmap.begin();iter!=hashmap.end();iter++){
            qDebug()<<"user:"<<iter.key()<<" state:"<<iter.value()->state();
        }
//        auto sockets=hashmap.values();
//        for(auto &socket:sockets){
//            socket->sendmsgs2client(10);
//        }
        if(hashmap.size()!=0)
            emit curServer->clientSendmsgs2client(10);
    });
//    setredis(Port.toInt(),anoname.toStdString().c_str());
}

CollServer::~CollServer(){
    // change set expire time 60 -> 10
    setexpire(Port.toInt(),AnoName.toStdString().c_str(),5);
    // recover port
    recoverPort(Port.toInt());
    std::cerr<<AnoName.toStdString()+" server is released\n";
    logfile->flush();

    while(list_thread.count()>0)
    {
        list_thread[0]->quit();
        list_thread[0]->wait();//等待退出
        list_thread[0]->deleteLater();//释放
        list_thread.removeAt(0);
    }

    exit(0);
}

CollServer* CollServer::getInstance(){
    return curServer;
}

void CollServer::incomingConnection(qintptr handle){

    setredis(Port.toInt(),AnoName.toStdString().c_str());
    list_thread.append(new CollThread(this));
    list_thread[list_thread.size()-1]->setServer(curServer);
    list_thread[list_thread.size()-1]->start();
//    std::this_thread::sleep_for(chrono::seconds(1));
    qDebug()<<"5";
//    emit list_thread[list_thread.size()-1]->sockethelper->create(handle);
    CollClient* client=new CollClient(handle,this);
    qDebug()<<"6";
    connect(client,&QTcpSocket::readyRead,client,&CollClient::onread);
    connect(client,&QTcpSocket::disconnected,client,&CollClient::ondisconnect);
    connect(client,&QAbstractSocket::errorOccurred,client,&CollClient::onError);
    connect(client,&CollClient::noUsers,this,&CollServer::imediateSave);
    connect(client,&CollClient::removeList,this,&CollServer::RemoveList);
    connect(client,&CollClient::exitNow,this,&CollServer::autoExit);
    connect(this,&CollServer::clientAddMarker,client,&CollClient::addmarkers);
    connect(this,&CollServer::clientSendMsgs,client,&CollClient::sendmsgs);
    connect(this,&CollServer::clientSendmsgs2client,client,&CollClient::sendmsgs2client);
    connect(this,&CollServer::clientUpdatesendmsgcnt,client,&CollClient::updatesendmsgcnt2processed);
    connect(this,&CollServer::clientDeleteLater,client,&CollClient::quit);
    connect(this,&CollServer::clientDisconnectFromHost,client,&CollClient::disconnectByServer);
    client->moveToThread(list_thread[list_thread.size()-1]);
//    new CollClient(handle,this);
}

void CollServer::imediateSave(){
    qDebug()<<"imediateSave";
    mutex.lock();
    savedmsgcnt=processedmsgcnt;
    mutex.unlock();
    writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(segments));
    writeAPO_file(Prefix+"/"+AnoName+".ano.apo",markers);
}

void CollServer::autoSave()
{
    std::cout<<"auto save\n"<<std::endl;
    logfile->flush();
    fsync(1);fsync(2);
    if(hashmap.size()==0){
        //没有用户了
        this->close();
        writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(segments));
        writeAPO_file(Prefix+"/"+AnoName+".ano.apo",markers);
        // 延迟析构对象
        deleteLater();
    }else{
        for (auto iter=hashmap.begin();iter!=hashmap.end();iter++){
            qDebug()<<"user:"<<iter.key()<<" state:"<<iter.value()->state();
        }
//        for(auto &socket:sockets){
//            socket->updatesendmsgcnt2processed();
//        }
        if(hashmap.size()!=0)
            emit clientUpdatesendmsgcnt();

        //        msglist.erase(msglist.begin(),msglist.begin()+processedmsgcnt);
        //        msglist.reserve(5000);
//        savedmsgcnt=processedmsgcnt;
        //        processedmsgcnt=0;
        //        msglist.erase(msglist.begin(),msglist.begin()+processedmsgcnt);
        //        msglist.reserve(5000);
        //        savedmsgcnt+=processedmsgcnt;
        //        processedmsgcnt=0;
        savedmsgcnt= processedmsgcnt;
        writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(segments));
        writeAPO_file(Prefix+"/"+AnoName+".ano.apo",markers);

    }
}

void CollServer::autoExit(){
//    auto sockets=hashmap.values();
//    for(auto &socket:sockets){
//        socket->deleteLater();
//    }
    emit clientDeleteLater();
    if(this->isListening())
        this->close();
    logfile->flush();
    deleteLater();
}

void CollServer::RemoveList(QThread* thread){
    qDebug()<<"thread:"<<thread<<"will be deleted";
    if(list_thread.size()!=0){
        for(int i=0;i<list_thread.size();i++)
        {
            if(thread==list_thread[i]){
                thread->quit();
                thread->wait();
                thread->deleteLater();
                list_thread.removeAt(i);
                qDebug()<<"list_thread.size()="<<list_thread.size();
                break;
            }
        }
    }
}

void CollServer::addmarkers(const QString msg){
    qDebug()<<msg;
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);
    if(headerlist.size()<2) {
        std::cerr<<"ERROR:headerlist.size<1\n";
    }

    unsigned int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }

    CellAPO marker;

    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=4) continue;
        marker.color.r=neuron_type_color[markerinfo[0].toUInt()][0];
        marker.color.g=neuron_type_color[markerinfo[0].toUInt()][1];
        marker.color.b=neuron_type_color[markerinfo[0].toUInt()][2];
        marker.x=markerinfo[1].toDouble();
        marker.y=markerinfo[2].toDouble();
        marker.z=markerinfo[3].toDouble();

        for(auto it=markers.begin();it!=markers.end(); ++it)
        {
            if(abs(it->x-marker.x)<1&&abs(it->y-marker.y)<1&&abs(it->z-marker.z)<1)
            {
                qDebug()<<"the marker has already existed";
                return;
            }
        }

        markers.append(marker);
        qDebug()<<"server addmarker";
    }
}

QString CollServer::getAnoName(){
    return AnoName;
}

QString CollServer::getImage(){
    return Image;
}

