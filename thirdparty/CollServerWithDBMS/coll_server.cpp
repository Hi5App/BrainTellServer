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
#include "grpcpp/grpcpp.h"
#include "Service/Service.grpc.pb.h"
#include "service/RpcCall.h"
#include "service/WrappedCall.h"
#include "config/config.h"
#include <filesystem>

extern QFile *logfile;

CollServer* CollServer::curServer=nullptr;

//QStringList CollServer::msglist;
//int CollServer::processedmsgcnt=0;
//int CollServer::savedmsgcnt=0;
//int CollServer::receivedcnt=0;
//QMap<QString,CollClient*> CollServer::hashmap;
//V_NeuronSWC_list CollServer::segments;
//QList<CellAPO> CollServer::markers;
////int receivedcnt;
//QString CollServer::swcpath;
//QString CollServer::apopath;
//QString CollServer::anopath;
//QMutex CollServer::mutex;
//QString CollServer::RES;

CollServer::CollServer(QString port,QString image,QString neuron,QString anoname,QString prefix,int maxUserNumsInt,int modelDetectIntervals,QObject *parent)
    :QTcpServer(parent),Port(port),Image(image),Neuron(neuron),AnoName(anoname),Prefix(prefix+"/testdata/"+image+"/"+neuron+"/"+anoname)
    ,MaxUserNums(maxUserNumsInt),ModelDetectIntervals(modelDetectIntervals),timerForAutoSave(new QTimer(this)),timerForDetectLoops(new QTimer(this)), timerForDetectOthers(new QTimer(this)),timerForDetectTip(new QTimer(this)),
    timerForDetectCrossing(new QTimer(this)),timerForAutoExit(new QTimer(this)),timerForDetectWhole(new QTimer(this))
{
    qDebug()<<"MainThread:"<<QThread::currentThreadId();
    curServer=this;

    Config::getInstance().initialize("config.json");
    Config::getInstance().readConfig();

    serverIP = Config::getInstance().getConfig(Config::ConfigItem::eServerIP);
    dbmsServerPort = Config::getInstance().getConfig(Config::ConfigItem::dbmsServerPort);
    brainServerPort = Config::getInstance().getConfig(Config::ConfigItem::brainServerPort);
    std::cout<<serverIP<<" "<<dbmsServerPort<<" "<<brainServerPort;

    detectUtil=new CollDetection(this, serverIP, brainServerPort, this);
//    string serverIP = "114.117.165.134";
//    string serverPort = "14251";
    auto endPoint = serverIP + ":" + dbmsServerPort;
    RpcCall::getInstance().initialize(endPoint);
    connectToDBMS();

    m_HeartBeatTimer = new QTimer(this);
    m_HeartBeatTimer->setInterval(15000);
    connect(m_HeartBeatTimer,&QTimer::timeout,this,[this]() {
        proto::UserOnlineHeartBeatNotification notification;
        notification.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
        auto* userInfo = notification.mutable_userverifyinfo();
        userInfo->set_username(cachedUserData.UserName);
        userInfo->set_usertoken(cachedUserData.UserToken);
        notification.set_heartbeattime(std::chrono::system_clock::now().time_since_epoch().count());
        proto::UserOnlineHeartBeatResponse response;
        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UserOnlineHeartBeatNotifications(&context,notification,&response);
        if(status.ok()) {
            cachedUserData.UserName = response.userverifyinfo().username();
            cachedUserData.UserToken = response.userverifyinfo().usertoken();
            cachedUserData.OnlineStatus = true;
        }else {
            qDebug()<<"Error" + QString::fromStdString(status.error_message());
        }
    });

    m_OnlineStatusTimer = new QTimer(this);
    m_OnlineStatusTimer->setInterval(30000);
    connect(m_OnlineStatusTimer,&QTimer::timeout,this,[this]() {
        if(!cachedUserData.OnlineStatus) {
            qDebug()<<"Error, Timeout! server may have disconnected from server!";
        }
        cachedUserData.OnlineStatus = false;
    });

    qRegisterMetaType<qintptr>("qintptr");

//    auto nt=readSWC_file(Prefix+"/"+AnoName+".ano.eswc");
//    segments=NeuronTree__2__V_NeuronSWC_list(nt);
//    markers=readAPO_file(Prefix+"/"+AnoName+".ano.apo");
    string dirpath=(Prefix+"/"+AnoName).toStdString();
    filesystem::create_directories(dirpath);

    swcpath=Prefix+"/"+AnoName+".ano.eswc";
    apopath=Prefix+"/"+AnoName+".ano.apo";
    anopath=Prefix+"/"+AnoName+".ano";

    tmp_swcpath = Prefix+"/"+AnoName+"_tmp.ano.eswc";
    tmp_apopath=Prefix+"/"+AnoName+"_tmp.ano.apo";
    tmp_anopath=Prefix+"/"+AnoName+"_tmp.ano";

    swcName = (AnoName + ".ano.eswc").toStdString();
    apoName = (AnoName + ".ano.apo").toStdString();

//    somaCoordinate=detectUtil->getSomaCoordinate(apopath);
//    detectUtil->getImageRES();
    // 3分钟执行一次
    timerForAutoSave->start(3*60*1000);
//    timerForDetectTip->setSingleShot(true);
    timerForAutoExit->start(20*60*60*1000);
    CollClient::timerforupdatemsg.start(1*1000);
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

    connect(timerForDetectLoops,&QTimer::timeout,detectUtil,&CollDetection::detectLoops);
    connect(timerForDetectOthers,&QTimer::timeout,detectUtil,&CollDetection::detectOthers);
    connect(timerForDetectWhole,&QTimer::timeout,detectUtil,&CollDetection::detectWholeAtStart);
    connect(timerForDetectTip,&QTimer::timeout,detectUtil,&CollDetection::detectTips);
    connect(timerForDetectCrossing,&QTimer::timeout,detectUtil,&CollDetection::detectCrossings);

    connect(timerForAutoSave,&QTimer::timeout,this,&CollServer::autoSave);
    connect(timerForAutoExit,&QTimer::timeout,this,&CollServer::autoExit);
    connect(&CollClient::timerforupdatemsg,&QTimer::timeout,[this]{
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
    startTimerForDetectWhole();
    m_HeartBeatTimer->start();
    m_OnlineStatusTimer->start();
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

    CollClient* client=new CollClient(handle,this);

    connect(client,&QTcpSocket::readyRead,client,&CollClient::onread);
    connect(client,&QTcpSocket::disconnected,client,&CollClient::ondisconnect);
    connect(client,&QAbstractSocket::errorOccurred,client,&CollClient::onError);
    connect(client,&CollClient::serverImediateSave,this,&CollServer::imediateSave);
    connect(client,&CollClient::removeList,this,&CollServer::RemoveList);
    connect(client,&CollClient::exitNow,this,&CollServer::autoExit);

    connect(client,&CollClient::serverStartTimerForDetectLoops,this,&CollServer::startTimerForDetectLoops);
    connect(client,&CollClient::serverStartTimerForDetectOthers,this,&CollServer::startTimerForDetectOthers);
    connect(client,&CollClient::serverStartTimerForDetectWhole,this,&CollServer::startTimerForDetectWhole);
    connect(client,&CollClient::serverStartTimerForDetectTip,this,&CollServer::startTimerForDetectTip);
    connect(client,&CollClient::serverStartTimerForDetectCrossing,this,&CollServer::startTimerForDetectCrossing);
    connect(client,&CollClient::detectUtilRemoveErrorSegs,detectUtil,&CollDetection::removeErrorSegs);

//    connect(this,&CollServer::clientAddMarker,client,&CollClient::addmarkers);
    connect(this,&CollServer::clientSendMsgs,client,&CollClient::sendmsgs);
    connect(this,&CollServer::clientSendmsgs2client,client,&CollClient::sendmsgs2client);
    connect(this,&CollServer::clientUpdatesendmsgcnt,client,&CollClient::updatesendmsgcnt2processed);
    connect(this,&CollServer::clientDeleteLater,client,&CollClient::quit);
    connect(this,&CollServer::clientDisconnectFromHost,client,&CollClient::disconnectByServer);
    client->moveToThread(list_thread[list_thread.size()-1]);
//    new CollClient(handle,this);
}

void CollServer::imediateSave(bool flag){
    qDebug()<<"imediateSave";
    savedmsgcnt=processedmsgcnt;
    writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(segments));
    writeAPO_file(Prefix+"/"+AnoName+".ano.apo",markers);
    if(flag)
        emit imediateSaveDone();
}

void CollServer::autoSave()
{
    std::cout<<"auto save\n"<<std::endl;
    logfile->flush();
    fsync(1);fsync(2);
    if(hashmap.size()==0){
        std::vector<proto::SwcAttachmentApoV1> swcAttachmentApoData;
        std::for_each(markers.begin(), markers.end(), [&](CellAPO&val) {
            proto::SwcAttachmentApoV1 data;
            data.set_n(val.n);
            data.set_orderinfo(val.orderinfo.toStdString());
            data.set_name(val.name.toStdString());
            data.set_comment(val.comment.toStdString());
            data.set_z(val.z);
            data.set_x(val.x);
            data.set_y(val.y);
            data.set_pixmax(val.pixmax);
            data.set_intensity(val.intensity);
            data.set_sdev(val.sdev);
            data.set_volsize(val.volsize);
            data.set_mass(val.mass);
            data.set_colorr(val.color.r);
            data.set_colorg(val.color.g);
            data.set_colorb(val.color.b);
            swcAttachmentApoData.push_back(data);
        });

        proto::UpdateSwcAttachmentApoResponse response;
        WrappedCall::updateSwcAttachmentApo(swcName, attachmentUuid, swcAttachmentApoData, response, cachedUserData);

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
        mutex.lock();
        savedmsgcnt= processedmsgcnt;
        mutex.unlock();
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

void CollServer::startTimerForDetectLoops(){
    timerForDetectLoops->start(24*60*60*1000);
}

void CollServer::startTimerForDetectOthers(){
    timerForDetectOthers->start(24*60*60*1000);
}

void CollServer::startTimerForDetectWhole(){
    timerForDetectWhole->setSingleShot(true);
    timerForDetectWhole->start(5*1000);
}

void CollServer::startTimerForDetectTip(){
    timerForDetectTip->start(ModelDetectIntervals*1000);
}

void CollServer::startTimerForDetectCrossing(){
    timerForDetectCrossing->start(24*60*60*1000);
}

bool CollServer::addmarkers(const QString msg){
    qDebug()<<msg;
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);

    int useridx=headerlist[0].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }

    CellAPO marker;
    marker.name="";
    marker.comment="";
    marker.orderinfo="";

    QMutexLocker locker(&this->mutex);
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=6) continue;
        marker.color.r=markerinfo[0].toUInt();
        marker.color.g=markerinfo[1].toUInt();
        marker.color.b=markerinfo[2].toUInt();
        marker.x=markerinfo[3].toDouble();
        marker.y=markerinfo[4].toDouble();
        marker.z=markerinfo[5].toDouble();

        for(auto it=markers.begin();it!=markers.end(); ++it)
        {
            if(abs(it->x-marker.x)<1&&abs(it->y-marker.y)<1&&abs(it->z-marker.z)<1)
            {
                qDebug()<<"the marker has already existed";
                return false;
            }
        }

        markers.append(marker);
        qDebug()<<"server addmarker";
        return true;
    }
}

QString CollServer::getAnoName(){
    return AnoName;
}

QString CollServer::getImage(){
    return Image;
}

QTimer* CollServer::getTimerForDetectLoops(){
    return timerForDetectLoops;
}

QTimer* CollServer::getTimerForDetectOthers(){
    return timerForDetectOthers;
}

QTimer* CollServer::getTimerForDetectTip(){
    return timerForDetectTip;
}

QTimer* CollServer::getTimerForDetectCrossing(){
    return timerForDetectCrossing;
}

QTimer* CollServer::getTimerForDetectWhole(){
    return timerForDetectWhole;
}

bool CollServer::connectToDBMS(){
    grpc::ClientContext context;
    proto::UserLoginRequest request;
    request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
    request.set_username(username.toStdString());
    request.set_password(password.toStdString());

    auto& rpcCall = RpcCall::getInstance();
    proto::UserLoginResponse response;
    auto status = rpcCall.Stub()->UserLogin(&context, request, &response);
    if(status.ok()){
        if(response.metainfo().status()) {
            auto timestampeNow = std::chrono::system_clock::now();
            std::chrono::days days(15);
            auto expiredTime = timestampeNow + days;
            auto seconds_since_epoch = expiredTime.time_since_epoch().count();

            cachedUserData.CachedUserMetaInfo = response.userinfo();
            cachedUserData.UserName = response.userverifyinfo().username();
            cachedUserData.UserToken = response.userverifyinfo().usertoken();
            cachedUserData.OnlineStatus = true;

            return true;
        }else {
            qDebug()<<"Server Login Failed!" + QString::fromStdString(response.metainfo().message());
            return false;
        }

    }else{
        qDebug()<<"Error" + QString::fromStdString(status.error_message());
    }
    return false;
}

