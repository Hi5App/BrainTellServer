#include "collclient.h"
#include "coll_server.h"
#include "utils.h"
#include <cmath>
#include <sort_swc.h>
#include <analyze.h>
#include <Message/Request.pb.h>
#include "service/RpcCall.h"
#include "service/WrappedCall.h"
#include "grpcpp/grpcpp.h"
#include "FileIo/AnoIo.hpp"
#include "FileIo/ApoIo.hpp"
#include "FileIo/FileIoInterface.hpp"
#include "FileIo/SwcIo.hpp"
//#include <algorithm>

extern QFile* logfile;


QTimer CollClient::timerforupdatemsg;

CollClient:: CollClient(qintptr handle, CollServer* curServer, QObject *parent):QTcpSocket(parent){
    setSocketDescriptor(handle);
    myServer=curServer;
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
            qDebug()<<"Error, Timeout! You may have disconnected from server!";
            QString msg = "/WARN_DisconnectError:server";
            sendmsgs({msg});
        }
        cachedUserData.OnlineStatus = false;
    });

//    如果一分钟内没有登陆好，则断开连接
    QTimer::singleShot(60*1000,this,[this]{
        if(userid==""){
            emit exitNow();
        }
    });
    // 检测客户端是否掉线，并作相应处理
    setSocketOption(QAbstractSocket::KeepAliveOption,1);//keepalive


}

void CollClient::updateuserlist()
{
    auto users=myServer->hashmap.keys();
    QString msg="/activeusers:"+users.join(',');
    for (auto iter=myServer->hashmap.begin();iter!=myServer->hashmap.end();iter++){
        qDebug()<<"user:"<<iter.key()<<" state:"<<iter.value()->state();
    }

//    emit myServer->clientSendMsgs({msg});

}

void CollClient::addseg(const QString msg)
{
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
    int isBegin=headerlist[2].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
        return;
    }
    XYZ point1,point2;

    QStringList pointlist_1=pointlist[0].split(' ',Qt::SkipEmptyParts);
    point1.x=pointlist_1[1].toFloat();
    point1.y=pointlist_1[2].toFloat();
    point1.z=pointlist_1[3].toFloat();
    for(int i=0;i<pointlist.size();i++)
    {
        if(pointlist[i]=="$"){
            QStringList pointlist_2=pointlist[i-1].split(' ',Qt::SkipEmptyParts);
            point2.x=pointlist_2[1].toFloat();
            point2.y=pointlist_2[2].toFloat();
            point2.z=pointlist_2[3].toFloat();
            break;
        }
    }

    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,1,clienttype);
    auto segs=NeuronTree__2__V_NeuronSWC_list(addnt).seg;
//    segs[0].printInfo();

//    bool flag;
//    if(fabs(point1.x-segs[0].row[0].x)<1e-4&&fabs(point1.y-segs[0].row[0].y)<1e-4&&fabs(point1.z-segs[0].row[0].z)<1e-4)
//        flag=true;
//    else
//        flag=false;

    QMutexLocker locker(&myServer->mutex);
//    myServer->segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
    bool isNeedReverse = false;
    if(segs.size()==2){
        int comparedIndex=0;
        if(isBegin==1){
            comparedIndex=segs[0].row.size()-1;
        }else if(isBegin==0){
            comparedIndex=0;
        }

        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),segs[1]);
//        it->printInfo();
        if(it!=myServer->segments.seg.end())
        {
            int index = -1;
            double mindist = 5;
            for(int i=0;i<it->row.size();i++){
                double dist=distance(segs[0].row[comparedIndex].x,it->row[i].x,segs[0].row[comparedIndex].y,it->row[i].y,segs[0].row[comparedIndex].z,it->row[i].z);
                if(dist<mindist)
                {
                    mindist=dist;
                    index=i;
                }
            }
            if(index == -1){
                qDebug()<<"INFO:cannot find nearest point in first connected seg";
                qDebug()<<segs[0].row[comparedIndex].x<<" "<<segs[0].row[comparedIndex].y<<" "<<segs[0].row[comparedIndex].z;
                it->printInfo();
            }
            else{
                segs[0].row[comparedIndex].x=it->row[index].x;
                segs[0].row[comparedIndex].y=it->row[index].y;
                segs[0].row[comparedIndex].z=it->row[index].z;

                if(comparedIndex==0)
                    isNeedReverse=true;
            }
            if(isNeedReverse)
                reverseSeg(segs[0]);
        }
        else
        {
            std::cerr<<"INFO:not find connected seg ,"<<msg.toStdString()<<std::endl;
        }
    }

    if(segs.size()==3){
//        set<size_t> segIds1;
//        set<size_t> segIds2;
//        int firstIndex = -1;
//        int firstEndIndex = -1;
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),segs[1]);
//        it->printInfo();
        if(it!=myServer->segments.seg.end())
        {
            int index = -1;
            double mindist = 5;
            for(int i=0;i<it->row.size();i++){
                double dist=distance(segs[0].row[segs[0].row.size()-1].x,it->row[i].x,segs[0].row[segs[0].row.size()-1].y,it->row[i].y,segs[0].row[segs[0].row.size()-1].z,it->row[i].z);
                if(dist<mindist)
                {
                    mindist=dist;
                    index=i;
                }
            }
            if(index == -1){
                qDebug()<<"INFO:cannot find nearest point in first connected seg";
                qDebug()<<segs[0].row[segs[0].row.size()-1].x<<" "<<segs[0].row[segs[0].row.size()-1].y<<" "<<segs[0].row[segs[0].row.size()-1].z;
                it->printInfo();
            }
            else{
                segs[0].row[segs[0].row.size()-1].x=it->row[index].x;
                segs[0].row[segs[0].row.size()-1].y=it->row[index].y;
                segs[0].row[segs[0].row.size()-1].z=it->row[index].z;
//                float xLabel = it->row[index].x;
//                float yLabel = it->row[index].y;
//                float zLabel = it->row[index].z;
//                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
//                string gridKey = gridKeyQ.toStdString();
//                map<string, set<size_t>> wholeGrid2SegIDMap = getWholeGrid2SegIDMap(myServer->segments);
//                segIds1 = wholeGrid2SegIDMap[gridKey];
//                firstIndex = index;
//                firstEndIndex = it->row.size()-1;
            }
        }
        else
        {
            std::cerr<<"INFO:not find connected seg ,"<<msg.toStdString()<<std::endl;
        }

        it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),segs[2]);
//        it->printInfo();
        if(it!=myServer->segments.seg.end())
        {
            int index = -1;
            double mindist = 5;
            for(int i=0;i<it->row.size();i++){
                double dist=distance(segs[0].row[0].x,it->row[i].x,segs[0].row[0].y,it->row[i].y,segs[0].row[0].z,it->row[i].z);
                if(dist<mindist)
                {
                    mindist=dist;
                    index=i;
                }
            }
            if(index == -1){
                qDebug()<<"INFO:cannot find nearest point in second connected seg";
                qDebug()<<segs[0].row[0].x<<" "<<segs[0].row[0].y<<" "<<segs[0].row[0].z;
                it->printInfo();
            }
            else{
                segs[0].row[0].x=it->row[index].x;
                segs[0].row[0].y=it->row[index].y;
                segs[0].row[0].z=it->row[index].z;
//                float xLabel = it->row[index].x;
//                float yLabel = it->row[index].y;
//                float zLabel = it->row[index].z;
//                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
//                string gridKey = gridKeyQ.toStdString();
//                map<string, set<size_t>> wholeGrid2SegIDMap = getWholeGrid2SegIDMap(myServer->segments);
//                segIds2 = wholeGrid2SegIDMap[gridKey];
            }
        }
        else
        {
            std::cerr<<"INFO:not find connected seg ,"<<msg.toStdString()<<std::endl;
        }

//        if(segIds1.size()==1 && segIds2.size()==1 && firstIndex==firstEndIndex && firstEndIndex!=-1 )
//            isNeedReverse = true;
//        if(segIds1.size()==1 && segIds2.size()>1)
//            isNeedReverse = true;
//        if(isNeedReverse)
//            reverseSeg(segs[0]);
    }

    V3DLONG point_size = myServer->segments.nrows();
    myServer->segments.append(segs[0]);

    myServer->mutexForDetectOthers.lock();
    myServer->last1MinSegments.append(segs[0]);
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    myServer->last3MinSegments.append(segs[0]);
    myServer->mutexForDetectMissing.unlock();

    proto::SwcDataV1 swcData;

    for(int i=0; i<segs[0].row.size(); i++){
        proto::SwcNodeInternalDataV1 swcNodeInternalData;
        swcNodeInternalData.set_n(point_size + i + 1);
        if(i == segs[0].row.size()-1)
            swcNodeInternalData.set_parent(-1);
        else
            swcNodeInternalData.set_parent(point_size + i + 2);
        swcNodeInternalData.set_x(segs[0].row[i].x);
        swcNodeInternalData.set_y(segs[0].row[i].y);
        swcNodeInternalData.set_z(segs[0].row[i].z);
        swcNodeInternalData.set_radius(segs[0].row[i].r);
        swcNodeInternalData.set_type(segs[0].row[i].type);
        swcNodeInternalData.set_mode(segs[0].row[i].creatmode);

        auto* newData = swcData.add_swcdata();
        newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
    }

    proto::CreateSwcNodeDataResponse response;
    if(!WrappedCall::addSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
        QString msg = "/WARN_AddSwcNodeDataError:server";
        sendmsgs({msg});
        return;
    }

    auto uuids = response.creatednodesuuid();
    for(int i=0; i<segs[0].row.size(); i++){
        myServer->segments.seg[myServer->segments.seg.size()-1].row[i].uuid = uuids.Get(i);
    }

    qDebug()<<"server addseg";
//    for(int i=0;i<myServer->segments.seg.size();i++){
//        myServer->segments.seg[i].printInfo();
//    }
}

void CollClient::addmanysegs(const QString msg){
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);
    if(headerlist.size()<2) {
        std::cerr<<"ERROR:headerlist.size<2\n";
    }
    unsigned int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
        return;
    }

    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,1,clienttype);
    auto segs=NeuronTree__2__V_NeuronSWC_list(addnt).seg;

    QMutexLocker locker(&myServer->mutex);
    myServer->mutexForDetectOthers.lock();
    myServer->mutexForDetectMissing.lock();

    for(auto seg:segs){
        myServer->last1MinSegments.append(seg);
        myServer->last3MinSegments.append(seg);
    }

    myServer->mutexForDetectMissing.unlock();
    myServer->mutexForDetectOthers.unlock();
    V3DLONG point_size = myServer->segments.nrows();

    int count=0;
    proto::SwcDataV1 swcData;
    for(auto seg:segs){
        for(int i=0; i<seg.row.size(); i++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_n(point_size + count + 1);
            if(i == seg.row.size()-1)
                swcNodeInternalData.set_parent(-1);
            else
                swcNodeInternalData.set_parent(point_size + count + 2);
            swcNodeInternalData.set_x(seg.row[i].x);
            swcNodeInternalData.set_y(seg.row[i].y);
            swcNodeInternalData.set_z(seg.row[i].z);
            swcNodeInternalData.set_radius(seg.row[i].r);
            swcNodeInternalData.set_type(seg.row[i].type);
            swcNodeInternalData.set_mode(seg.row[i].creatmode);

            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            count++;
        }
    }

    proto::CreateSwcNodeDataResponse response;
    if(!WrappedCall::addSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
        QString msg = "/WARN_AddSwcNodeDataError:server";
        sendmsgs({msg});
        return;
    }

    count = 0;
    auto uuids = response.creatednodesuuid();
    for(int i=0; i<segs.size(); i++){
        for(int j=0; j<segs[i].row.size(); j++){
            segs[i].row[j].uuid = uuids.Get(count);
            count++;
        }
        myServer->segments.append(segs[i]);
    }

    qDebug()<<"server addmanysegs";
}

void CollClient::delseg(const QString msg)
{
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);
    if(headerlist.size()<2) {
        std::cerr<<"ERROR:headerlist.size<1\n";
    }
    int useridx=headerlist[1].toUInt();
    unsigned int clienttype=headerlist[0].toUInt();
    unsigned int isMany=0;
    if(headerlist.size()>=6)
        isMany=headerlist[5].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
        return;
    }
    auto delnt=convertMsg2NT(pointlist,clienttype,useridx,isMany,clienttype);
    auto delsegs=NeuronTree__2__V_NeuronSWC_list(delnt).seg;

    int count=0;
    QMutexLocker locker(&myServer->mutex);

//    for(int i=0; i<myServer->segments.seg.size(); i++)
//        myServer->segments.seg[i].printInfo();
    proto::SwcDataV1 swcData;
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),delsegs[i]);

        if(it!=myServer->segments.seg.end())
        {
            for(int j=0; j<it->row.size(); j++){
                proto::SwcNodeInternalDataV1 swcNodeInternalData;
                swcNodeInternalData.set_x(it->row[j].x);
                swcNodeInternalData.set_y(it->row[j].y);
                swcNodeInternalData.set_z(it->row[j].z);
                swcNodeInternalData.set_radius(it->row[j].r);
                swcNodeInternalData.set_type(it->row[j].type);
                swcNodeInternalData.set_mode(it->row[j].creatmode);

                auto* newData = swcData.add_swcdata();
                newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
                newData->mutable_base()->set_uuid(it->row[j].uuid);
            }
            myServer->segments.seg.erase(it);
            if(count<5)
                qDebug()<<"server delseg";
            count++;
        }
        else{
            std::cerr<<"INFO:not find del seg"<<std::endl;
            delsegs[i].printInfo();
        }
    }

    proto::DeleteSwcNodeDataResponse response;
//    if(!WrappedCall::deleteSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
//        QString msg = "/WARN_DeleteSwcNodeDataError:server";
//        sendmsgs({msg});
//        return;
//    }
    WrappedCall::deleteSwcNodeData(myServer->swcName, swcData, response, cachedUserData);

    myServer->mutexForDetectOthers.lock();
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->last1MinSegments.seg.begin(),myServer->last1MinSegments.seg.end(),delsegs[i]);
        if(it!=myServer->last1MinSegments.seg.end())
        {
            myServer->last1MinSegments.seg.erase(it);
        }
    }
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->last3MinSegments.seg.begin(),myServer->last3MinSegments.seg.end(),delsegs[i]);
        if(it!=myServer->last3MinSegments.seg.end())
        {
            myServer->last3MinSegments.seg.erase(it);
        }
    }
    myServer->mutexForDetectMissing.unlock();

//    for(int i=0;i<myServer->segments.seg.size();i++){
//        myServer->segments.seg[i].printInfo();
//    }
}

void CollClient::connectseg(const QString msg){
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
        return;
    }

    QStringList specPointsInfo1=pointlist[0].split(' ',Qt::SkipEmptyParts);
    QStringList specPointsInfo2=pointlist[1].split(' ',Qt::SkipEmptyParts);
    XYZ p1=XYZ(specPointsInfo1[0].toFloat(), specPointsInfo1[1].toFloat(), specPointsInfo1[2].toFloat());
    XYZ p2=XYZ(specPointsInfo2[0].toFloat(), specPointsInfo2[1].toFloat(), specPointsInfo2[2].toFloat());
    pointlist.removeAt(0);
    pointlist.removeAt(0);

    auto segnt=convertMsg2NT(pointlist,clienttype,useridx,1,clienttype);
    auto connectsegs=NeuronTree__2__V_NeuronSWC_list(segnt).seg;
//    for(int i=0;i<connectsegs.size();i++){
//        connectsegs[i].printInfo();
//    }

    vector<segInfoUnit> segInfo;

    QMutexLocker locker(&myServer->mutex);
    for(int i=0;i<connectsegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),connectsegs[i]);
        if(it!=myServer->segments.seg.end())
        {
            //父子关系逆序
            if (it->row.begin()->data[6] != 2) // Sort the node numbers of involved segments
            {
                int nodeNo = 1;
                for (vector<V_NeuronSWC_unit>::iterator it_unit = it->row.begin();
                     it_unit != it->row.end(); it_unit++)
                {
                    it_unit->data[0] = nodeNo;
                    it_unit->data[6] = nodeNo + 1;
                    ++nodeNo;
                }
                (it->row.end() - 1)->data[6] = -1;
            }

            //构造segInfo
            if(segInfo.size()==0){
                double mindist=5;
                vector<V_NeuronSWC_unit>::iterator it_res=it->row.end();
                for (vector<V_NeuronSWC_unit>::iterator it_unit = it->row.begin();
                     it_unit != it->row.end(); it_unit++)
                {
                    //if (p1.x == it_unit->data[2] && p1.y == it_unit->data[3] && p1.z == it_unit->data[4])
                    double dist=distance(p1.x,it_unit->data[2],p1.y,it_unit->data[3],p1.z,it_unit->data[4]);
                    if(dist<mindist)
                    {
                        mindist=dist;
                        it_res=it_unit;
                    }
                }
                if(it_res==it->row.end()){
                    qDebug()<<"cannot find nearest point in first to be connected seg";
                    qDebug()<<p1.x<<" "<<p1.y<<" "<<p1.z;
                    it->printInfo();
                }
                else{
                    //---------------------- Get seg IDs
                    //qDebug() << nodeOnStroke->at(j).seg_id << " " << nodeOnStroke->at(j).parent << " " << p.x() << " " << p.y();
                    qDebug()<<p1.x<<" "<<p1.y<<" "<<p1.z;
                    qDebug()<<it_res->data[2]<<" "<<it_res->data[3]<<" "<<it_res->data[4];
                    segInfoUnit curSeg;
                    curSeg.head_tail = it_res->data[6];
                    curSeg.segID = it-myServer->segments.seg.begin();
                    curSeg.nodeCount = it->row.size();
                    curSeg.refine = false;
                    curSeg.branchID = it->branchingProfile.ID;
                    curSeg.paBranchID = it->branchingProfile.paID;
                    curSeg.hierarchy = it->branchingProfile.hierarchy;
                    segInfo.push_back(curSeg);
                }

            }
            else if(segInfo.size()==1){
                double mindist=5;
                vector<V_NeuronSWC_unit>::iterator it_res=it->row.end();
                for (vector<V_NeuronSWC_unit>::iterator it_unit = it->row.begin();
                     it_unit != it->row.end(); it_unit++)
                {
                    double dist=distance(p2.x,it_unit->data[2],p2.y,it_unit->data[3],p2.z,it_unit->data[4]);
                    if(dist<mindist)
                    {
                        mindist=dist;
                        it_res=it_unit;
                    }
                }

                if(it_res==it->row.end()){
                    qDebug()<<"cannot find nearest point in second to be connected seg";
                    qDebug()<<p2.x<<" "<<p2.y<<" "<<p2.z;
                    it->printInfo();
                }
                else{
                    //---------------------- Get seg IDs
                    //qDebug() << nodeOnStroke->at(j).seg_id << " " << nodeOnStroke->at(j).parent << " " << p.x() << " " << p.y();
                    qDebug()<<p2.x<<" "<<p2.y<<" "<<p2.z;
                    qDebug()<<it_res->data[2]<<" "<<it_res->data[3]<<" "<<it_res->data[4];
                    segInfoUnit curSeg;
                    curSeg.head_tail = it_res->data[6];
                    curSeg.segID = it-myServer->segments.seg.begin();
                    curSeg.nodeCount = it->row.size();
                    curSeg.refine = false;
                    curSeg.branchID = it->branchingProfile.ID;
                    curSeg.paBranchID = it->branchingProfile.paID;
                    curSeg.hierarchy = it->branchingProfile.hierarchy;
                    segInfo.push_back(curSeg);
                    qDebug()<<"second connect seg found";
                }

            }

        }

        else{
            std::cerr<<"INFO:not find connect seg ,"<<msg.toStdString()<<std::endl;
//            myServer->mutex.unlock();
            return;
        }
    }

    simpleConnectExecutor(myServer->segments, segInfo);

    int point_size = myServer->segments.nrows();
    vector<V_NeuronSWC> connectedSegDecomposed;
    if (myServer->segments.seg[segInfo[0].segID].to_be_deleted)
    {
        qDebug()<<"enter tracedNeuron.seg[segInfo[0]]";
        connectedSegDecomposed = decompose_V_NeuronSWC(myServer->segments.seg[segInfo[1].segID]);
        qDebug()<<"connectedSegDecomposed_size: "<<connectedSegDecomposed.size();
//        for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
//            myServer->segments.seg.push_back(*addedIt);

        myServer->segments.seg[segInfo[1].segID].to_be_deleted = true;
        myServer->segments.seg[segInfo[1].segID].on = false;

    }
    else if (myServer->segments.seg[segInfo[1].segID].to_be_deleted)
    {
        qDebug()<<"enter tracedNeuron.seg[segInfo[1]]";
        connectedSegDecomposed = decompose_V_NeuronSWC(myServer->segments.seg[segInfo[0].segID]);
//        for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
//            myServer->segments.seg.push_back(*addedIt);

        myServer->segments.seg[segInfo[0].segID].to_be_deleted = true;
        myServer->segments.seg[segInfo[0].segID].on = false;
    }

    proto::SwcDataV1 swcData;
    int count=0;
    for(auto seg:connectedSegDecomposed){

        for(int i=0; i<seg.row.size(); i++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_n(point_size + count + 1);
            if(i == seg.row.size()-1)
                swcNodeInternalData.set_parent(-1);
            else
                swcNodeInternalData.set_parent(point_size + count + 2);
            swcNodeInternalData.set_x(seg.row[i].x);
            swcNodeInternalData.set_y(seg.row[i].y);
            swcNodeInternalData.set_z(seg.row[i].z);
            swcNodeInternalData.set_radius(seg.row[i].r);
            swcNodeInternalData.set_type(seg.row[i].type);
            swcNodeInternalData.set_mode(seg.row[i].creatmode);
            count++;

            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
        }
    }

    proto::CreateSwcNodeDataResponse response;
    if(!WrappedCall::addSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
        QString msg = "/WARN_AddSwcNodeDataError:server";
        sendmsgs({msg});
        return;
    }

    count = 0;
    auto uuids = response.creatednodesuuid();
    for(int i=0; i<connectedSegDecomposed.size(); i++){
        for(int j=0; j<connectedSegDecomposed[i].row.size(); j++){
            connectedSegDecomposed[i].row[j].uuid = uuids.Get(count);
            count++;
        }
        myServer->segments.append(connectedSegDecomposed[i]);
    }

    std::vector<V_NeuronSWC>::iterator iter = myServer->segments.seg.begin();
    while (iter != myServer->segments.seg.end())
        if (iter->to_be_deleted){
            auto seg = *iter;
            proto::SwcDataV1 swcData;

            for(int i=0; i<seg.row.size(); i++){
                proto::SwcNodeInternalDataV1 swcNodeInternalData;
                swcNodeInternalData.set_x(seg.row[i].x);
                swcNodeInternalData.set_y(seg.row[i].y);
                swcNodeInternalData.set_z(seg.row[i].z);
                swcNodeInternalData.set_radius(seg.row[i].r);
                swcNodeInternalData.set_type(seg.row[i].type);
                swcNodeInternalData.set_mode(seg.row[i].creatmode);

                auto* newData = swcData.add_swcdata();
                newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
                newData->mutable_base()->set_uuid(seg.row[i].uuid);
            }

            proto::DeleteSwcNodeDataResponse response;
            if(!WrappedCall::deleteSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
                QString msg = "/WARN_DeleteSwcNodeDataError:server";
                sendmsgs({msg});
                return;
            }
            iter = myServer->segments.seg.erase(iter);
        }
        else
            ++iter;

//    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,0);
//    myServer->segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
    qDebug()<<"server connectseg";
}

void CollClient::splitseg(const QString msg){
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);
    if(headerlist.size()<2) {
        std::cerr<<"ERROR:headerlist.size<1\n";
    }
    int useridx=headerlist[1].toUInt();
    unsigned int clienttype=headerlist[0].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
        return;
    }

    auto tempnt=convertMsg2NT(pointlist,clienttype,useridx,1,clienttype);
    auto segs=NeuronTree__2__V_NeuronSWC_list(tempnt).seg;

    if(segs.size()<=2)
        return;

    XYZ point1,point2;

    QMutexLocker locker(&myServer->mutex);
    auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),segs[0]);
    if(it!=myServer->segments.seg.end())
    {
        point1.x=it->row[0].x;
        point1.y=it->row[0].y;
        point1.z=it->row[0].z;
        point2.x=it->row[it->row.size()-1].x;
        point2.y=it->row[it->row.size()-1].y;
        point2.z=it->row[it->row.size()-1].z;
        proto::SwcDataV1 swcData;

        for(int i=0; i<it->row.size(); i++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_x(it->row[i].x);
            swcNodeInternalData.set_y(it->row[i].y);
            swcNodeInternalData.set_z(it->row[i].z);
            swcNodeInternalData.set_radius(it->row[i].r);
            swcNodeInternalData.set_type(it->row[i].type);
            swcNodeInternalData.set_mode(it->row[i].creatmode);

            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            newData->mutable_base()->set_uuid(it->row[i].uuid);
        }

        proto::DeleteSwcNodeDataResponse response;
        if(!WrappedCall::deleteSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
            QString msg = "/WARN_DeleteSwcNodeDataError:server";
            sendmsgs({msg});
            return;
        }
        myServer->segments.seg.erase(it);
    }

    else
    {
        std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
    }

    int point_size = myServer->segments.nrows();
    int count=0;
    myServer->mutexForDetectOthers.lock();
    auto it_last1Min=findseg(myServer->last1MinSegments.seg.begin(),myServer->last1MinSegments.seg.end(),segs[0]);
    if(it_last1Min!=myServer->last1MinSegments.seg.end())
    {
        myServer->last1MinSegments.seg.erase(it_last1Min);
    }
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    auto it_last3Min=findseg(myServer->last3MinSegments.seg.begin(),myServer->last3MinSegments.seg.end(),segs[0]);
    if(it_last3Min!=myServer->last3MinSegments.seg.end())
    {
        myServer->last3MinSegments.seg.erase(it_last3Min);
    }
    myServer->mutexForDetectMissing.unlock();


    proto::SwcDataV1 swcData;
    for(int i=1;i<segs.size();i++){
        if(distance(segs[i].row[0].x,point1.x,segs[i].row[0].y,point1.y,segs[i].row[0].z,point1.z)<0.3){
            segs[i].row[0].x=point1.x;
            segs[i].row[0].y=point1.y;
            segs[i].row[0].z=point1.z;
        }
        if(distance(segs[i].row[0].x,point2.x,segs[i].row[0].y,point2.y,segs[i].row[0].z,point2.z)<0.3){
            segs[i].row[0].x=point2.x;
            segs[i].row[0].y=point2.y;
            segs[i].row[0].z=point2.z;
            if(segs[i].row[0].parent!=-1){
                reverse(segs[i].row.begin(),segs[i].row.end());
                //父子关系逆序
                int nodeNo = 1;
                for (vector<V_NeuronSWC_unit>::iterator it_unit = segs[i].row.begin();
                     it_unit != segs[i].row.end(); it_unit++)
                {
                    it_unit->data[0] = nodeNo;
                    it_unit->data[6] = nodeNo + 1;
                    ++nodeNo;
                }
                (segs[i].row.end() - 1)->data[6] = -1;
            }
        }
        if(distance(segs[i].row[segs[i].row.size()-1].x,point2.x,segs[i].row[segs[i].row.size()-1].y,point2.y,segs[i].row[segs[i].row.size()-1].z,point2.z)<0.3){
            segs[i].row[segs[i].row.size()-1].x=point2.x;
            segs[i].row[segs[i].row.size()-1].y=point2.y;
            segs[i].row[segs[i].row.size()-1].z=point2.z;
        }
        if(distance(segs[i].row[segs[i].row.size()-1].x,point1.x,segs[i].row[segs[i].row.size()-1].y,point1.y,segs[i].row[segs[i].row.size()-1].z,point1.z)<0.3){
            segs[i].row[segs[i].row.size()-1].x=point1.x;
            segs[i].row[segs[i].row.size()-1].y=point1.y;
            segs[i].row[segs[i].row.size()-1].z=point1.z;
            if(segs[i].row[0].parent!=-1){
                reverse(segs[i].row.begin(),segs[i].row.end());
                //父子关系逆序
                int nodeNo = 1;
                for (vector<V_NeuronSWC_unit>::iterator it_unit = segs[i].row.begin();
                     it_unit != segs[i].row.end(); it_unit++)
                {
                    it_unit->data[0] = nodeNo;
                    it_unit->data[6] = nodeNo + 1;
                    ++nodeNo;
                }
                (segs[i].row.end() - 1)->data[6] = -1;
            }
        }

        for(int j=0; j<segs[i].row.size(); j++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_n(point_size + count + 1);
            if(j == segs[i].row.size()-1)
                swcNodeInternalData.set_parent(-1);
            else
                swcNodeInternalData.set_parent(point_size + count + 2);
            swcNodeInternalData.set_x(segs[i].row[j].x);
            swcNodeInternalData.set_y(segs[i].row[j].y);
            swcNodeInternalData.set_z(segs[i].row[j].z);
            swcNodeInternalData.set_radius(segs[i].row[j].r);
            swcNodeInternalData.set_type(segs[i].row[j].type);
            swcNodeInternalData.set_mode(segs[i].row[j].creatmode);

            count++;
            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
        }
    }

    proto::CreateSwcNodeDataResponse response;
    if(!WrappedCall::addSwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
        QString msg = "/WARN_AddSwcNodeDataError:server";
        sendmsgs({msg});
        return;
    }

    count = 0;
    auto uuids = response.creatednodesuuid();
    for(int i=1; i<segs.size(); i++){
        for(int j=0; j<segs[i].row.size(); j++){
            segs[i].row[j].uuid = uuids.Get(count);
            count++;
        }
        myServer->segments.append(segs[i]);
    }

    myServer->mutexForDetectOthers.lock();
    for(int i=1;i<segs.size();i++){
        myServer->last1MinSegments.append(segs[i]);
    }
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    for(int i=1;i<segs.size();i++){
        myServer->last3MinSegments.append(segs[i]);
    }
    myServer->mutexForDetectMissing.unlock();

    qDebug()<<"server splitseg";
}

void CollClient::retypesegment(const QString msg)
{
    QStringList pointlistwithheader=msg.split(',',Qt::SkipEmptyParts);
    if(pointlistwithheader.size()<1){
        std::cerr<<"ERROR:pointlistwithheader.size<1\n";
    }

    QStringList headerlist=pointlistwithheader[0].split(' ',Qt::SkipEmptyParts);
    if(headerlist.size()<3) {
        std::cerr<<"ERROR:headerlist.size<1\n";
    }

    unsigned int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    unsigned int newcolor=headerlist[2].toUInt();
    unsigned int isMany=0;
    if(headerlist.size()>=7)
        isMany=headerlist[6].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
        return;
    }

    auto retypent=convertMsg2NT(pointlist,clienttype,useridx,isMany,clienttype);
    auto retypesegs=NeuronTree__2__V_NeuronSWC_list(retypent).seg;

    int count=0;
    proto::SwcDataV1 swcData;
    QMutexLocker locker(&myServer->mutex);
    for(int i=0;i<retypesegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),retypesegs[i]);
        if(it==myServer->segments.seg.end()){
            std::cerr<<"INFO:not find retype seg ,"<<msg.toStdString()<<std::endl;
            //            myServer->mutex.unlock();
            continue;
        }
        int now=QDateTime::currentMSecsSinceEpoch();
        for(auto &unit:it->row){
            unit.type=newcolor;
            unit.level=now-unit.timestamp;
            unit.creatmode=useridx*10+clienttype;
        }

        for(int j=0; j<it->row.size(); j++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_x(it->row[j].x);
            swcNodeInternalData.set_y(it->row[j].y);
            swcNodeInternalData.set_z(it->row[j].z);
            swcNodeInternalData.set_radius(it->row[j].r);
            swcNodeInternalData.set_type(it->row[j].type);
            swcNodeInternalData.set_mode(it->row[j].creatmode);

            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            newData->mutable_base()->set_uuid(it->row[j].uuid);
        }

        if(count<5)
            qDebug()<<"server retypesegment";
        count++;
    }

    proto::UpdateSwcNodeDataResponse response;
    if(!WrappedCall::modifySwcNodeData(myServer->swcName, swcData, response, cachedUserData)){
        QString msg = "/WARN_ModifySwcNodeDataError:server";
        sendmsgs({msg});
        return;
    }
}

void CollClient::addmarkers(const QString msg)
{
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
        return;
    }

    CellAPO marker;
    marker.name="";
    marker.comment="";
    marker.orderinfo="";

    QMutexLocker locker(&myServer->mutex);
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=6) continue;
        marker.color.r=markerinfo[0].toUInt();
        marker.color.g=markerinfo[1].toUInt();
        marker.color.b=markerinfo[2].toUInt();
        marker.x=markerinfo[3].toDouble();
        marker.y=markerinfo[4].toDouble();
        marker.z=markerinfo[5].toDouble();

        for(auto it=myServer->markers.begin();it!=myServer->markers.end(); ++it)
        {
            if(fabs(it->x-marker.x)<1&&fabs(it->y-marker.y)<1&&fabs(it->z-marker.z)<1)
            {
                qDebug()<<"the marker has already existed";
//                myServer->mutex.unlock();
                return;
            }
        }

        myServer->markers.append(marker);
        qDebug()<<"server addmarker";
    }
}

void CollClient::delmarkers(const QString msg)
{
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
        return;
    }
    CellAPO marker;
    int idx=-1;

    QMutexLocker locker(&myServer->mutex);
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=6) continue;
        marker.color.r=markerinfo[0].toUInt();
        marker.color.g=markerinfo[1].toUInt();
        marker.color.b=markerinfo[2].toUInt();
        marker.x=markerinfo[3].toDouble();
        marker.y=markerinfo[4].toDouble();
        marker.z=markerinfo[5].toDouble();
//        if(myServer->isSomaExists&&sqrt((marker.x-myServer->somaCoordinate.x)*(marker.x-myServer->somaCoordinate.x)+
//                (marker.y-myServer->somaCoordinate.y)*(marker.y-myServer->somaCoordinate.y)+
//                (marker.z-myServer->somaCoordinate.z)*(marker.z-myServer->somaCoordinate.z))<1)
//        {
//            qDebug()<<"cannot delete the soma marker";
////            myServer->mutex.unlock();
//            return;
//        }
        idx=findnearest(marker,myServer->markers);
        if(idx!=-1) {
            myServer->markers.removeAt(idx);
            qDebug()<<"server delmarker";
        }
        else{
            std::cerr<<"find marker failed."+msg.toStdString()+"\n";
        }
    }
}

void CollClient::retypemarker(const QString msg){
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
        return;
    }
    CellAPO marker;
    int idx=-1;

    QMutexLocker locker(&myServer->mutex);
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=6) continue;
        marker.color.r=markerinfo[0].toUInt();
        marker.color.g=markerinfo[1].toUInt();
        marker.color.b=markerinfo[2].toUInt();
        marker.x=markerinfo[3].toDouble();
        marker.y=markerinfo[4].toDouble();
        marker.z=markerinfo[5].toDouble();
//        if(isSomaExists&&sqrt((marker.x-somaCoordinate.x)*(marker.x-somaCoordinate.x)+
//                                 (marker.y-somaCoordinate.y)*(marker.y-somaCoordinate.y)+
//                                 (marker.z-somaCoordinate.z)*(marker.z-somaCoordinate.z))<1)
//        {
//            qDebug()<<"cannot delete the soma marker";
//            return;
//        }
        idx=findnearest(marker,myServer->markers);
        if(idx!=-1) {
            myServer->markers[idx].color.r=marker.color.r;
            myServer->markers[idx].color.g=marker.color.g;
            myServer->markers[idx].color.b=marker.color.b;
            qDebug()<<"server retypemarker";
        }
        else{
            std::cerr<<"find marker failed."+msg.toStdString()+"\n";
        }
    }
}



void CollClient::sendmsgs(const QStringList &msgs)
{
//    if(!this) return;
    if(this->state()!=QAbstractSocket::ConnectedState){
        qDebug()<<"error: send msg to "<<this->userid<<",but connect is "<<this->state();
//        ondisconnect();
        return;
    }
    qDebug()<<"server send msgs";

    const std::string data=msgs.join(';').toStdString();
    const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
    // QString::fromStdString(header)将header转换为utf-8编码的字符串
    write(header.c_str(),header.size());
    write(data.c_str(),data.size());
    this->flush();
}

void CollClient::sendfiles(const QStringList &filepaths, const QStringList& filenames)
{
    if(this->state()!=QAbstractSocket::ConnectedState)
        return;

    if(filepaths.size()!=filenames.size())
        return;

    int count=0;
    for(const auto &path:filepaths){
        QString filename=filenames[count];
        QFile f(path);
        if(f.open(QIODevice::ReadOnly)){
            QByteArray data=f.readAll();
            const std::string header=QString("DataTypeWithSize:%1 %2 %3\n").arg(1).arg(filename.size()).arg(data.size()).toStdString();
            write(header.c_str(),header.size());
            write(filename.toStdString().c_str(),filename.size());
            write(data,data.size());
        }
        count++;
    }
    this->flush();

}

void CollClient::preprocessmsgs(const QStringList &msgs)
{
    for(int i=0;i<msgs.size();i++){
        auto msg=msgs[i];
        if(msg.contains("/login:"))
        {
            auto ps=msg.right(msg.size()-QString("/login:").size()).split(' ',Qt::SkipEmptyParts);
            if (ps.size()>5){
                std::cerr<<"login in error:"<<msg.toStdString();
//                this->disconnectFromHost();
//                this->close();//关闭读
                emit exitNow();
                return;
            }
            qDebug()<<"subThread:"<<QThread::currentThreadId();

            userid=ps[0];
            username=ps[1];
            password=ps[2];
            if(!myServer->hashmap.contains(username)&&myServer->currentUserNum>=myServer->MaxUserNums)
            {
                QString msg = "/WARN_FullNumberError:server";
                sendmsgs({msg});
                emit myServer->clientDisconnectFromHost(this);
                return;
            }

            bool result=connectToDBMS();
            if(result){
                m_HeartBeatTimer->start();
                m_OnlineStatusTimer->start();
            }else{
                QString msg = "/WARN_DisconnectError:server";
                sendmsgs({msg});
                emit myServer->clientDisconnectFromHost(this);
                return;
            }
            myServer->mutex.lock();
            receiveuser(ps[1], ps[2], ps[3], myServer->isFirstClient);
            updateApoData(myServer->isFirstClient);
            getFileFromDBMSAndSend(myServer->isFirstClient);
            myServer->isFirstClient = false;
            myServer->mutex.unlock();
            startCollaborate();

        }else if(msg.contains("/ANALYZE")){
            if(msg.startsWith("/ANALYZE_SomaNearBy:")){
                analyzeSomaNearBy(msg.right(msg.size()-QString("/ANALYZE_SomaNearBy:").size()));
            }
            else if(msg.startsWith("/ANALYZE_ColorMutation:")){
                analyzeColorMutation(msg.right(msg.size()-QString("/ANALYZE_ColorMutation:").size()));
            }
            else if(msg.startsWith("/ANALYZE_Dissociative:")){
                analyzeDissociativeSegs(msg.right(msg.size()-QString("/ANALYZE_Dissociative:").size()));
            }
            else if(msg.startsWith("/ANALYZE_Angle:")){
                analyzeAngles(msg.right(msg.size()-QString("/ANALYZE_Angle:").size()));
            }
        }else if(msg.contains("/DEFINE")){
            if(msg.startsWith("/DEFINE_Soma:")){
                defineSoma(msg.right(msg.size()-QString("/DEFINE_Soma:").size()));
            }
        }else if(msg.contains("/SEND")){
            if(msg.startsWith("/SEND_SomaPos:")){
                getSomaPos(msg.right(msg.size()-QString("/SEND_SomaPos:").size()));
            }
        }
        else{
            if(msg.startsWith("/drawline_norm:")||msg.startsWith("/drawline_undo:")||msg.startsWith("/drawline_redo:")){
                addseg(msg.right(msg.size()-QString("/drawline_norm:").size()));
            }else if(msg.startsWith("/delline_norm:")||msg.startsWith("/delline_undo:")||msg.startsWith("/delline_redo:")){
                delseg(msg.right(msg.size()-QString("/delline_norm:").size()));
            }else if(msg.startsWith("/addmarker_norm:")||msg.startsWith("/addmarker_undo:")||msg.startsWith("/addmarker_redo:")){
                addmarkers(msg.right(msg.size()-QString("/addmarker_norm:").size()));
            }else if(msg.startsWith("/delmarker_norm:")||msg.startsWith("/delmarker_undo:")||msg.startsWith("/delmarker_redo:")){
                delmarkers(msg.right(msg.size()-QString("/delmarker_norm:").size()));
            }else if(msg.startsWith("/retypemarker_norm:")||msg.startsWith("/retypemarker_undo:")||msg.startsWith("/retypemarker_redo:")){
                retypemarker(msg.right(msg.size()-QString("/retypemarker_norm:").size()));
            }else if(msg.startsWith("/connectline_norm:")||msg.startsWith("/connectline_undo:")||msg.startsWith("/connectline_redo:")){
                connectseg(msg.right(msg.size()-QString("/connectline_norm:").size()));
            }else if(msg.startsWith("/retypeline_norm:")||msg.startsWith("/retypeline_undo:")||msg.startsWith("/retypeline_redo:")){
                retypesegment(msg.right(msg.size()-QString("/retypeline_norm:").size()));
            }else if(msg.startsWith("/splitline_norm:")||msg.startsWith("/splitline_undo:")||msg.startsWith("/splitline_redo:")){
                splitseg(msg.right(msg.size()-QString("/splitline_norm:").size()));
            }else if(msg.startsWith("/drawmanylines_norm:")||msg.startsWith("/drawmanylines_undo:")){
                addmanysegs(msg.right(msg.size()-QString("/drawmanylines_norm:").size()));
            }

            myServer->mutex.lock();
            myServer->processedmsgcnt+=1;

            QString log;
            // QString::number按照第二个参数提供的转换进制将数字类型转换为QString
            log=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")+QString::number(myServer->processedmsgcnt+myServer->savedmsgcnt)+" "+msg+"\n";

            logfile->write(log.toStdString().c_str(),log.toStdString().size());
            myServer->msglist.append(msg);
            myServer->mutex.unlock();
        }
    }

}

void CollClient::onread()
{
    while(1){
        if(!datatype.isFile)
        {
            //不是准备接受文件数据
            if(datatype.datasize==0){
                //准备接收数据头
                if(this->canReadLine()){
                    QString msg=readLine(1024).trimmed();
                    qDebug()<<QString(msg).toStdString().c_str();
                    if(!msg.startsWith("DataTypeWithSize:")){
                        this->write("Socket Receive ERROR!");
                        // 这里要进行相应处理后break吧
                        std::cerr<<userid.toStdString()+" receive not match format\n";
                        emit exitNow();
                    }

                    auto ps=msg.right(msg.size()-QString("DataTypeWithSize:").size()).split(' ');
                    if (ps[0].toInt()!=0){
                        this->write("Socket Receive ERROR!");
                        std::cerr<<userid.toStdString()+" receive not match format\n";
                        emit exitNow();
                    }
                    datatype.isFile=ps[0].toUInt();
                    datatype.datasize=ps[1].toUInt();
                }else{
                    break;
                }
            }else{
                //已经接收了头，正在准备接收 消息数据
                // bytesAvailable返回可供读取的字节数。
                if(bytesAvailable()>=datatype.datasize){
                    char *data=new char[datatype.datasize+1];
                    this->read(data,datatype.datasize);
                    data[datatype.datasize]='\0';

                    myServer->mutex.lock();
                    myServer->receivedcnt+=1;
                    myServer->mutex.unlock();

                    QString log;
                    log=QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ") + QString::number(myServer->receivedcnt) + " receive from " + userid + " :" + QString(data);

                    qDebug()<<log;
//                    qDebug()<<QString("client read message %1, %2").arg(username).arg(data).toStdString().c_str();
                    preprocessmsgs(QString(data).trimmed().split(';',Qt::SkipEmptyParts));
                    resetdatatype();
                    delete [] data;
                }else{
                    break;
                }
            }
        }else{
            break;
        }
    }

}

void CollClient::ondisconnect()
{
    qDebug()<<errorString();
    qDebug()<<username<<QString(" client disconnect").toStdString().c_str();
    this->flush();
    int count=0;
    while(this->bytesAvailable()){
        count++;
        if(count>10)
            break;
        onread();
    }
    qDebug()<<"ondisconnect: 00";
    this->close();//关闭读
    myServer->mutex.lock();
    if(myServer->hashmap.contains(username)&&myServer->hashmap[username]==this)
        myServer->hashmap.remove(username);
    if(myServer->hashmap.size()==0)
    {
        emit serverImediateSave(false);
    }
    myServer->currentUserNum -= 1;
    myServer->mutex.unlock();
    updateuserlist();
    qDebug()<<"ondisconnect: 11";
    qDebug()<<"subthread "<<QThread::currentThreadId()<<" will quit";
    emit removeList(thread());
    this->deleteLater();  
}

void CollClient::onError(QAbstractSocket::SocketError socketError){
    const QMetaObject & mo = QAbstractSocket::staticMetaObject;
    QMetaEnum me = mo.enumerator(mo.indexOfEnumerator("SocketError"));
    qDebug()<<me.valueToKey(socketError);
    qDebug()<<username<<QString(" client occurs error").toStdString().c_str();
//    this->flush();
//    while(this->bytesAvailable())
//        onread();
//    this->close();//关闭读
//    if(CollClient::hashmap.contains(username)&&CollClient::hashmap[username]==this)
//        CollClient::hashmap.remove(username);
//    if(CollClient::hashmap.size()==0)
//    {
//        curClient=this;
//        emit noUsers();
//    }
//    CollClient::updateuserlist();
//    this->deleteLater();
}

void CollClient::receiveuser(const QString userName, QString passWord, QString RES, bool isFirstClient)
{
    if(myServer->hashmap.contains(userName))
    {
        std::cerr<<"ERROR:"+userName.toStdString()+" is duolicate,will remove the first\n";
        emit myServer->clientDisconnectFromHost(myServer->hashmap[userName]);
    }
    else{
        myServer->currentUserNum += 1;
    }
    myServer->hashmap[userName]=this;
    myServer->RES=RES;
    updateuserlist();

    if(!isFirstClient){
        // 创建事件循环
        QEventLoop loop;

        // 连接信号和事件循环的退出槽
        connect(myServer, &CollServer::imediateSaveDone, &loop, &QEventLoop::quit);

        emit serverImediateSave(true);

        // 等待事件循环退出
        loop.exec();
    }

    sendmsgcnt=myServer->savedmsgcnt;
    qDebug()<<"receive user init sendmsgcnt = "<<sendmsgcnt;
}

void CollClient::getFileFromDBMSAndSend(bool isFirstClient){
    string anoName = (myServer->getAnoName()+".ano").toStdString();

    //get ano
    std::filesystem::path anoPath(myServer->tmp_anopath.toStdString());
    AnoIo anoIo(anoPath.string());
    AnoUnit unit;
    unit.APOFILE = myServer->apoName;
    unit.SWCFILE = myServer->swcName;
    anoIo.setValue(unit);
    anoIo.WriteToFile();

    //get apo
    proto::GetSwcMetaInfoResponse get_swc_meta_info_response;
    if (!WrappedCall::getSwcMetaInfoByName(myServer->swcName, get_swc_meta_info_response, cachedUserData)) {
        QString msg = "/WARN_GetSwcMetaInfoError:server";
        sendmsgs({msg});
        return;
    }

    if (get_swc_meta_info_response.swcinfo().swcattachmentapometainfo().attachmentuuid().empty())
    {
        qDebug()<<"No Apo Attachment found! You can create a new apo attchment!";
        QString msg = "/WARN_ApoFileNotFoundError:server";
        sendmsgs({msg});
        return;
    }

    proto::GetSwcAttachmentApoResponse response;
    myServer->attachmentUuid = get_swc_meta_info_response.swcinfo().swcattachmentapometainfo().attachmentuuid();
    if (!WrappedCall::getSwcAttachmentApo(myServer->swcName, myServer->attachmentUuid, response, cachedUserData)) {
        QString msg = "/WARN_GetApoDataError:server";
        sendmsgs({msg});
        return;
    }

    vector<proto::SwcAttachmentApoV1> m_SwcAttachmentApoData;

    for (auto&data: response.swcattachmentapo()) {
        m_SwcAttachmentApoData.push_back(data);
    }

    std::filesystem::path apoPath(myServer->tmp_apopath.toStdString());
    ApoIo apoIo(apoPath.string());
    std::vector<ApoUnit> units;
    std::for_each(m_SwcAttachmentApoData.begin(), m_SwcAttachmentApoData.end(),
                  [&](proto::SwcAttachmentApoV1&val) {
                      ApoUnit unit;
                      unit.n = val.n();
                      unit.orderinfo = val.orderinfo();
                      unit.name = val.name();
                      unit.comment = val.comment();
                      unit.z = val.z();
                      unit.x = val.x();
                      unit.y = val.y();
                      unit.pixmax = val.pixmax();
                      unit.intensity = val.intensity();
                      unit.sdev = val.sdev();
                      unit.volsize = val.volsize();
                      unit.mass = val.mass();
                      unit.color_r = val.colorr();
                      unit.color_g = val.colorg();
                      unit.color_b = val.colorb();
                      units.push_back(unit);
                  });

    apoIo.setValue(units);
    apoIo.WriteToFile();

    //get eswc
    proto::GetSwcMetaInfoResponse response1;
    if(!WrappedCall::getSwcMetaInfoByName(myServer->swcName,response1,cachedUserData)){
        QString msg = "/WARN_GetSwcMetaInfoError:server";
        sendmsgs({msg});
        return;
    }

    proto::GetSwcFullNodeDataResponse response2;
    if(!WrappedCall::getSwcFullNodeData(myServer->swcName, response2, cachedUserData)){
        QString msg = "/WARN_GetSwcFullNodeDataError:server";
        sendmsgs({msg});
        return;
    }

    ExportSwcData exportSwcData;
    exportSwcData.swcData = response2.swcnodedata();
    exportSwcData.swcMetaInfo = response1.swcinfo();
    vector<string> uuidVec;

    std::filesystem::path swcPath(myServer->tmp_swcpath.toStdString());
    if(exportSwcData.swcMetaInfo.swctype() == "swc"){
        vector<NeuronUnit> neurons;
        auto swcData = exportSwcData.swcData;
        for (int j = 0; j < swcData.swcdata_size(); j++) {
            NeuronUnit unit;
            unit.n = swcData.swcdata(j).swcnodeinternaldata().n();
            unit.type = swcData.swcdata(j).swcnodeinternaldata().type();
            unit.x = swcData.swcdata(j).swcnodeinternaldata().x();
            unit.y = swcData.swcdata(j).swcnodeinternaldata().y();
            unit.z = swcData.swcdata(j).swcnodeinternaldata().z();
            unit.radius = swcData.swcdata(j).swcnodeinternaldata().radius();
            unit.parent = swcData.swcdata(j).swcnodeinternaldata().parent();
            neurons.push_back(unit);
            uuidVec.push_back(swcData.swcdata(j).base().uuid());
        }

        Swc swc(swcPath.string());
        swc.setValue(neurons);
        swc.WriteToFile();
    }

    else if(exportSwcData.swcMetaInfo.swctype() == "eswc"){
        std::vector<NeuronUnit> neurons;
        auto swcData = exportSwcData.swcData;
        for (int j = 0; j < swcData.swcdata_size(); j++) {
            NeuronUnit unit;
            unit.n = swcData.swcdata(j).swcnodeinternaldata().n();
            unit.type = swcData.swcdata(j).swcnodeinternaldata().type();
            unit.x = swcData.swcdata(j).swcnodeinternaldata().x();
            unit.y = swcData.swcdata(j).swcnodeinternaldata().y();
            unit.z = swcData.swcdata(j).swcnodeinternaldata().z();
            unit.radius = swcData.swcdata(j).swcnodeinternaldata().radius();
            unit.parent = swcData.swcdata(j).swcnodeinternaldata().parent();
            unit.seg_id = swcData.swcdata(j).swcnodeinternaldata().seg_id();
            unit.level = swcData.swcdata(j).swcnodeinternaldata().level();
            unit.mode = swcData.swcdata(j).swcnodeinternaldata().mode();
            unit.timestamp = swcData.swcdata(j).swcnodeinternaldata().timestamp();
            unit.feature_value = swcData.swcdata(j).swcnodeinternaldata().feature_value();
            neurons.push_back(unit);
            uuidVec.push_back(swcData.swcdata(j).base().uuid());
        }

        ESwc eSwc(swcPath.string());
        eSwc.setValue(neurons);
        eSwc.WriteToFile();
    }

    if(isFirstClient){
        auto nt=readSWC_file(myServer->tmp_swcpath);
        myServer->segments=NeuronTree__2__V_NeuronSWC_list(nt, uuidVec);
        myServer->markers=readAPO_file(myServer->tmp_apopath);
        myServer->somaCoordinate=myServer->detectUtil->getSomaCoordinate(myServer->tmp_apopath);
    }

    //发送保存的文件
    sendfiles({myServer->tmp_anopath,myServer->tmp_apopath,myServer->tmp_swcpath},
              {QString::fromStdString(anoName),QString::fromStdString(myServer->apoName),QString::fromStdString(myServer->swcName)});

    QFile file(myServer->tmp_anopath);
    if(file.remove()){
        qDebug()<<"tmp ano file removed successfully";
    }else{
        qDebug()<<"error removing tmp ano file: "<<file.errorString();
    }

    file.setFileName(myServer->tmp_apopath);
    if(file.remove()){
        qDebug()<<"tmp apo file removed successfully";
    }else{
        qDebug()<<"error removing tmp apo file: "<<file.errorString();
    }

    file.setFileName(myServer->tmp_swcpath);
    if(file.remove()){
        qDebug()<<"tmp swc file removed successfully";
    }else{
        qDebug()<<"error removing tmp swc file: "<<file.errorString();
    }

}

void CollClient::updateApoData(bool isFirstClient){
    if(isFirstClient)
        return;

    std::vector<proto::SwcAttachmentApoV1> swcAttachmentApoData;
    std::for_each(myServer->markers.begin(), myServer->markers.end(), [&](CellAPO&val) {
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
    if(!WrappedCall::updateSwcAttachmentApo(myServer->swcName, myServer->attachmentUuid, swcAttachmentApoData, response, cachedUserData)){
        QString msg = "/WARN_UpdateSwcAttachmentApoError:server";
        sendmsgs({msg});
        return;
    }
}

void CollClient::startCollaborate(){
    // 获取协同的ano文件名
    QString msg=QString("STARTCOLLABORATE:%1").arg(myServer->anopath.section('/',-1,-1));
    sendmsgs({msg});

    if(!myServer->getTimerForDetectLoops()->isActive())
        emit serverStartTimerForDetectLoops();
    if(!myServer->getTimerForDetectOthers()->isActive())
        emit serverStartTimerForDetectOthers();
    //    if(!myServer->getTimerForDetectTip()->isActive())
    //        emit serverStartTimerForDetectTip();
    if(!myServer->getTimerForDetectCrossing()->isActive())
        emit serverStartTimerForDetectCrossing();
}

void CollClient::updatesendmsgcnt2processed()
{
    if(sendmsgcnt<myServer->processedmsgcnt)
    {
        sendmsgs(QStringList(myServer->msglist.begin()+this->sendmsgcnt,
                             myServer->msglist.begin()+myServer->processedmsgcnt));
        sendmsgcnt=myServer->processedmsgcnt;
    }
//    sendmsgcnt-=CollClient::processedmsgcnt;
}

void CollClient::sendmsgs2client(int maxsize)
{
    if(myServer->msglist.size()<=sendmsgcnt){
        qDebug()<<"msglist.size="<<myServer->msglist.size()<<" sendmsgcnt="<<sendmsgcnt;
        return;
    }
    auto end=MIN(int(myServer->msglist.size()),int(this->sendmsgcnt+maxsize));
//    if(maxsize>0)
//        maxsize=MIN(maxsize,CollClient::msglist.size()-sendmsgcnt);
//    else
//        maxsize=CollClient::msglist.size()-sendmsgcnt;
//    qDebug()<<"send to "<< this->username<<" :("<<myServer->msglist.begin()+this->sendmsgcnt
//           <<","<<myServer->msglist.begin()+end<<")/"<<myServer->msglist.size();
    //左闭右开
    sendmsgs(QStringList(myServer->msglist.begin()+this->sendmsgcnt,
                         myServer->msglist.begin()+end));
//    this->sendmsgcnt+=maxsize;
    this->sendmsgcnt=end;
}

void CollClient::resetdatatype()
{
    datatype.isFile=false;
    datatype.datasize=0;
}

void CollClient::quit(){
    this->deleteLater();
}

void CollClient::disconnectByServer(CollClient* collclient){
    if(collclient==this){
        qDebug()<<"client will disconnectFromHost";
        this->disconnectFromHost();
    }
}

void CollClient::simpleConnectExecutor(V_NeuronSWC_list& segments, vector<segInfoUnit>& segInfo)
{

    qDebug()<<"begin to simpleConnectExecutor";
    // This method is the "executor" of Renderer_gl1::simpleConnect(), MK, May, 2018
    segInfoUnit mainSeg, branchSeg;
    bool flag=false;
    V_NeuronSWC old_seg;
    V_NeuronSWC res_seg;
    int point_size=0;

    //////////////////////////////////////////// HEAD TAIL CONNECTION ////////////////////////////////////////////
    if ((segInfo.at(0).head_tail == -1 || segInfo.at(0).head_tail == 2) && (segInfo.at(1).head_tail == -1 || segInfo.at(1).head_tail == 2))
    {
        flag=true;
        if (segInfo.at(0).nodeCount >= segInfo.at(1).nodeCount)
        {
            mainSeg = segInfo.at(0);
            branchSeg = segInfo.at(1);
            qDebug() << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail;
            qDebug() << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail;
        }
        else
        {
            mainSeg = segInfo.at(1);
            branchSeg = segInfo.at(0);
            qDebug() << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail;
            qDebug() << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail;
        }

        double assignedType;
        assignedType = segments.seg[segInfo.at(0).segID].row[0].type;
        segments.seg[mainSeg.segID].row[0].seg_id = mainSeg.segID;
        old_seg=segments.seg[mainSeg.segID];
        point_size = segments.nrows()-old_seg.nrows();
        //        qDebug()<<"zll___debug__mainSeg.head_tail"<<mainSeg.head_tail;
        if (mainSeg.head_tail == -1)
        {
            //            qDebug()<<"(zll-debug)branchSeg.head_tail="<<branchSeg.head_tail;
            if (branchSeg.head_tail == -1) // head to head
            {
                for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[branchSeg.segID].row.end() - 1;
                     itNextSeg >= segments.seg[branchSeg.segID].row.begin(); --itNextSeg)
                {
                    itNextSeg->seg_id = branchSeg.segID;
                    segments.seg[mainSeg.segID].row.push_back(*itNextSeg);
                }
            }
            else if (branchSeg.head_tail == 2) // head to tail
            {
                for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[branchSeg.segID].row.begin();
                     itNextSeg != segments.seg[branchSeg.segID].row.end(); ++itNextSeg)
                {
                    itNextSeg->seg_id = branchSeg.segID;
                    segments.seg[mainSeg.segID].row.push_back(*itNextSeg);
                }
            }
            segments.seg[branchSeg.segID].to_be_deleted = true;
            segments.seg[branchSeg.segID].on = false;

            // sorting the new segment here, and reassign the root node to the new tail
            size_t nextSegNo = 1;
            for (vector<V_NeuronSWC_unit>::iterator itSort = segments.seg[mainSeg.segID].row.begin();
                 itSort != segments.seg[mainSeg.segID].row.end(); ++itSort)
            {
                itSort->data[0] = nextSegNo;
                itSort->data[6] = itSort->data[0] + 1;
                ++nextSegNo;
            }
            (segments.seg[mainSeg.segID].row.end() - 1)->data[6] = -1;
        }
        else if (mainSeg.head_tail == 2)
        {
            std::reverse(segments.seg[mainSeg.segID].row.begin(), segments.seg[mainSeg.segID].row.end());
            //            qDebug()<<"zll___debug__2_branchSeg.head_tail"<<branchSeg.head_tail;
            if (branchSeg.head_tail == -1) // tail to head
            {
                for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[branchSeg.segID].row.end() - 1;
                     itNextSeg >= segments.seg[branchSeg.segID].row.begin(); itNextSeg--)
                {
                    itNextSeg->seg_id = branchSeg.segID;
                    segments.seg[mainSeg.segID].row.push_back(*itNextSeg);
                }
            }
            else if (branchSeg.head_tail == 2) // tail to tail
            {
                for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[branchSeg.segID].row.begin();
                     itNextSeg != segments.seg[branchSeg.segID].row.end(); itNextSeg++)
                {
                    itNextSeg->seg_id = branchSeg.segID;
                    segments.seg[mainSeg.segID].row.push_back(*itNextSeg);
                }
            }
            segments.seg[branchSeg.segID].to_be_deleted = true;
            segments.seg[branchSeg.segID].on = false;

            // sorting the new segment here, and reassign the root node to the new tail
            std::reverse(segments.seg[mainSeg.segID].row.begin(), segments.seg[mainSeg.segID].row.end());
            size_t nextSegNo = 1;
            for (vector<V_NeuronSWC_unit>::iterator itSort = segments.seg[mainSeg.segID].row.begin();
                 itSort != segments.seg[mainSeg.segID].row.end(); itSort++)
            {
                itSort->data[0] = nextSegNo;
                itSort->data[6] = itSort->data[0] + 1;
                ++nextSegNo;
            }
            (segments.seg[mainSeg.segID].row.end() - 1)->data[6] = -1;
        }

        // correcting types, based on the main segment type
        for (vector<V_NeuronSWC_unit>::iterator reID = segments.seg[mainSeg.segID].row.begin();
             reID != segments.seg[mainSeg.segID].row.end(); ++reID)
        {
            reID->seg_id = mainSeg.segID;
            reID->type = assignedType;
            //            qDebug()<<"zll_debug"<<reID->type;
        }
        res_seg=segments.seg[mainSeg.segID];
    }
    //////////////////////////////////////////// END of [HEAD TAIL CONNECTION] ////////////////////////////////////////////

    //////////////////////////////////////////// BRANCHING CONNECTION ////////////////////////////////////////////
    if ((segInfo.at(0).head_tail != -1 && segInfo.at(0).head_tail != 2) ^ (segInfo.at(1).head_tail != -1 && segInfo.at(1).head_tail != 2))
    {
        flag=true;
        if (segInfo.at(0).head_tail == -1 || segInfo.at(0).head_tail == 2)
        {
            mainSeg = segInfo.at(1);
            branchSeg = segInfo.at(0);
            qDebug() << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail;
            qDebug() << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail;
        }
        else
        {
            mainSeg = segInfo.at(0);
            branchSeg = segInfo.at(1);
            qDebug() << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail;
            qDebug() << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail;
        }

        double assignedType;
        assignedType = segments.seg[segInfo.at(0).segID].row[0].type;
        segments.seg[mainSeg.segID].row[0].seg_id = mainSeg.segID;
        old_seg=segments.seg[mainSeg.segID];
        point_size = segments.nrows()-old_seg.nrows();
        if (branchSeg.head_tail == 2) // branch to tail
        {
            std::reverse(segments.seg[branchSeg.segID].row.begin(), segments.seg[branchSeg.segID].row.end());
            size_t branchSegLength = segments.seg[branchSeg.segID].row.size();
            size_t mainSegLength = segments.seg[mainSeg.segID].row.size();
            segments.seg[mainSeg.segID].row.insert(segments.seg[mainSeg.segID].row.end(), segments.seg[branchSeg.segID].row.begin(), segments.seg[branchSeg.segID].row.end());
            size_t branchN = mainSegLength + 1;
            for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[mainSeg.segID].row.end() - 1;
                 itNextSeg != segments.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSegLength - 1); --itNextSeg)
            {
                itNextSeg->n = branchN;
                itNextSeg->seg_id = mainSeg.segID;
                itNextSeg->parent = branchN - 1;
                ++branchN;
            }
            (segments.seg[mainSeg.segID].row.end() - 1)->parent = (segments.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSeg.head_tail - 2))->n;
            segments.seg[branchSeg.segID].to_be_deleted = true;
            segments.seg[branchSeg.segID].on = false;
        }
        else if (branchSeg.head_tail == -1) // branch to head
        {
            size_t branchSegLength = segments.seg[branchSeg.segID].row.size();
            size_t mainSegLength = segments.seg[mainSeg.segID].row.size();
            segments.seg[mainSeg.segID].row.insert(segments.seg[mainSeg.segID].row.end(), segments.seg[branchSeg.segID].row.begin(), segments.seg[branchSeg.segID].row.end());
            size_t branchN = mainSegLength + 1;
            for (vector<V_NeuronSWC_unit>::iterator itNextSeg = segments.seg[mainSeg.segID].row.end() - 1;
                 itNextSeg != segments.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSegLength - 1); --itNextSeg)
            {
                itNextSeg->n = branchN;
                itNextSeg->seg_id = mainSeg.segID;
                itNextSeg->parent = branchN - 1;
                ++branchN;
            }
            (segments.seg[mainSeg.segID].row.end() - 1)->parent = (segments.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSeg.head_tail - 2))->n;
            segments.seg[branchSeg.segID].to_be_deleted = true;
            segments.seg[branchSeg.segID].on = false;
        }

        // correcting types, based on the main segment type
        for (vector<V_NeuronSWC_unit>::iterator reID = segments.seg[mainSeg.segID].row.begin();
             reID != segments.seg[mainSeg.segID].row.end(); ++reID)
        {
            reID->seg_id = mainSeg.segID;
            reID->type = assignedType;
        }
        res_seg=segments.seg[mainSeg.segID];
    }
    //////////////////////////////////////////// END of [BRANCHING CONNECTION] ////////////////////////////////////////////
    if(flag){
        proto::SwcDataV1 deleteSwcData;

        for(int i=0; i<old_seg.row.size(); i++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_x(old_seg.row[i].x);
            swcNodeInternalData.set_y(old_seg.row[i].y);
            swcNodeInternalData.set_z(old_seg.row[i].z);
            swcNodeInternalData.set_radius(old_seg.row[i].r);
            swcNodeInternalData.set_type(old_seg.row[i].type);
            swcNodeInternalData.set_mode(old_seg.row[i].creatmode);

            auto* newData = deleteSwcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            newData->mutable_base()->set_uuid(old_seg.row[i].uuid);
        }

        proto::DeleteSwcNodeDataResponse deleteResponse;
        if(!WrappedCall::deleteSwcNodeData(myServer->swcName, deleteSwcData, deleteResponse, cachedUserData)){
            QString msg = "/WARN_DeleteSwcNodeDataError:server";
            sendmsgs({msg});
            return;
        }

        proto::SwcDataV1 addSwcData;

        for(int i=0; i<res_seg.row.size(); i++){
            proto::SwcNodeInternalDataV1 swcNodeInternalData;
            swcNodeInternalData.set_n(point_size + i + 1);
            if(i == res_seg.row.size()-1)
                swcNodeInternalData.set_parent(-1);
            else
                swcNodeInternalData.set_parent(point_size + i + 2);
            swcNodeInternalData.set_x(res_seg.row[i].x);
            swcNodeInternalData.set_y(res_seg.row[i].y);
            swcNodeInternalData.set_z(res_seg.row[i].z);
            swcNodeInternalData.set_radius(res_seg.row[i].r);
            swcNodeInternalData.set_type(res_seg.row[i].type);
            swcNodeInternalData.set_mode(res_seg.row[i].creatmode);

            auto* newData = addSwcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
        }

        proto::CreateSwcNodeDataResponse addResponse;
        if(!WrappedCall::addSwcNodeData(myServer->swcName, addSwcData, addResponse, cachedUserData)){
            QString msg = "/WARN_AddSwcNodeDataError:server";
            sendmsgs({msg});
            return;
        }

        auto uuids = addResponse.creatednodesuuid();
        for(int i=0; i<res_seg.row.size(); i++){
            myServer->segments.seg[mainSeg.segID].row[i].uuid = uuids.Get(i);
        }
    }

    return;
}

void CollClient::analyzeSomaNearBy(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("analyzeSomaNearBy: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    if(!myServer->isSomaExists){
        qDebug()<<"soma not detected!";
        QString tobeSendMsg="/FEEDBACK_ANALYZE_SomaNearBy:";
        tobeSendMsg += QString("server %1").arg(-1);
        sendmsgs({tobeSendMsg});
        return;
    }
    else{
        myServer->mutex.lock();
        myServer->mutexForDetectOthers.lock();
        myServer->mutexForDetectMissing.lock();
        // 创建事件循环
        QEventLoop loop;

        // 连接信号和事件循环的退出槽
        connect(myServer->detectUtil, &CollDetection::removeErrorSegsDone, &loop, &QEventLoop::quit);

        emit detectUtilRemoveErrorSegs(true);

        // 等待事件循环退出
        loop.exec();
        myServer->mutexForDetectMissing.unlock();
        myServer->mutexForDetectOthers.unlock();
        myServer->mutex.unlock();
        vector<int> counts=getMulfurcationsCountNearSoma(8, myServer->somaCoordinate, myServer->segments);
        QString tobeSendMsg="/FEEDBACK_ANALYZE_SomaNearBy:";
        if((counts[1] + counts[2])==1){
            qDebug()<<"the soma has been connected to one point";
            tobeSendMsg += QString("server %1").arg(1);
        }else if((counts[1] + counts[2])==0 && counts[0]==1){
            qDebug()<<"the soma has been connected to one point";
            tobeSendMsg += QString("server %1").arg(1);
        }else{
            qDebug()<<"the soma is not connected to one point!";
            tobeSendMsg += QString("server %1").arg(0);
        }
        sendmsgs({tobeSendMsg});
    }
}

void CollClient::analyzeColorMutation(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("analyzeColorMutation: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    bool result=true;
    if(!myServer->isSomaExists){
        qDebug()<<"soma not detected!";
        QString tobeSendMsg="/FEEDBACK_ANALYZE_ColorMutation:";
        tobeSendMsg += QString("server %1").arg(-1);
        sendmsgs({tobeSendMsg});
        return;
    }
    else{
        map<string,set<int>> specPointsMap=getColorChangedPoints(myServer->segments);
        set<string> resultSet;
        for(auto it=specPointsMap.begin();it!=specPointsMap.end();it++){
            int size = 0;
            for(auto it2=it->second.begin(); it2!=it->second.end(); it2++){
                if(*it2!=2 && *it2!=3 && *it2!=4 && *it2!=1)
                    size++;
            }
            if(size!=0){
                resultSet.insert(it->first);
            }
        }
        for(auto it=resultSet.begin();it!=resultSet.end();){
            NeuronSWC s;
            stringToXYZ(*it, s.x, s.y, s.z);
            if(distance(s.x, myServer->somaCoordinate.x, s.y, myServer->somaCoordinate.y,
                         s.z, myServer->somaCoordinate.z)<8)
            {
                it=resultSet.erase(it);
            }else{
                it++;
            }
        }
        if(resultSet.size()!=0){
            QString tobeSendMsg="/FEEDBACK_ANALYZE_ColorMutation:";
            qDebug()<<"color mutation exists";
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg +=",";
            for(auto it=resultSet.begin(); it!=resultSet.end(); it++){
                NeuronSWC s;
                stringToXYZ(*it, s.x, s.y, s.z);
                tobeSendMsg += QString("%1 %2 %3 %4 %5 %6").arg(200).arg(20).arg(0).arg(s.x).arg(s.y).arg(s.z);
                tobeSendMsg += ",";
            }
            tobeSendMsg.chop(1);
            sendmsgs({tobeSendMsg});
            return;
        }

        int case_type=0;
        for(auto it=specPointsMap.begin();it!=specPointsMap.end();it++){
            if(it->second.find(2)!=it->second.end() && it->second.find(3)!=it->second.end()){
                resultSet.insert(it->first);
            }
        }
        if(resultSet.size()!=1)
            result=false;
        else{
            string gridKey=*resultSet.begin();
            XYZ coor;
            stringToXYZ(gridKey, coor.x, coor.y, coor.z);
            if(distance(coor.x, myServer->somaCoordinate.x, coor.y, myServer->somaCoordinate.y,
                         coor.z, myServer->somaCoordinate.z)<8)
                case_type=1;
            else
                case_type=2;
        }

        if(case_type==0){
            qDebug()<<"axons not connected or the number of axons is more than 1";
        }
        else if(case_type==1){
            for(auto it=specPointsMap.begin();it!=specPointsMap.end();it++){
                if(it->second.find(2)!=it->second.end() && it->second.find(4)!=it->second.end()){
                    resultSet.insert(it->first);
                }
                if(it->second.find(4)!=it->second.end() && it->second.find(3)!=it->second.end()){
                    resultSet.insert(it->first);
                }
            }
            if(resultSet.size()!=1)
                result=false;
        }
        else if(case_type==2){
            string gridKey;
            for(auto it=specPointsMap.begin();it!=specPointsMap.end();it++){
                if(it->second.find(4)!=it->second.end() && it->second.find(3)!=it->second.end()){
                    resultSet.insert(it->first);
                    gridKey=it->first;
                }
            }
            if(resultSet.size()>2)
            {
                result=false;
            }
            else if(resultSet.size()==2){
                XYZ coor;
                stringToXYZ(gridKey, coor.x, coor.y, coor.z);
                if(distance(coor.x, myServer->somaCoordinate.x, coor.y, myServer->somaCoordinate.y,
                             coor.z, myServer->somaCoordinate.z)>8)
                    result=false;
            }

            int curCount=resultSet.size();
            for(auto it=specPointsMap.begin();it!=specPointsMap.end();it++){
                if(it->second.find(4)!=it->second.end() && it->second.find(2)!=it->second.end()){
                    resultSet.insert(it->first);
                }
            }
            if(curCount!=resultSet.size())
                result=false;

        }

        for(auto it=resultSet.begin();it!=resultSet.end();){
            NeuronSWC s;
            stringToXYZ(*it, s.x, s.y, s.z);
            if(distance(s.x, myServer->somaCoordinate.x, s.y, myServer->somaCoordinate.y,
                         s.z, myServer->somaCoordinate.z)<8)
            {
                it=resultSet.erase(it);
            }else{
                it++;
            }
        }

        QString tobeSendMsg="/FEEDBACK_ANALYZE_ColorMutation:";
        if(result){
            qDebug()<<"no color mutation";
            tobeSendMsg += QString("server %1").arg(1);
        }else{
            qDebug()<<"color mutation exists";
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg +=",";
            for(auto it=resultSet.begin(); it!=resultSet.end(); it++){
                NeuronSWC s;
                stringToXYZ(*it, s.x, s.y, s.z);
                tobeSendMsg += QString("%1 %2 %3 %4 %5 %6").arg(200).arg(20).arg(0).arg(s.x).arg(s.y).arg(s.z);
                tobeSendMsg += ",";
            }
            tobeSendMsg.chop(1);
        }
        sendmsgs({tobeSendMsg});
    }
}

void CollClient::analyzeDissociativeSegs(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("analyzeDissociativeSegs: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    set<string> dissociativePoints=getDissociativeSegEndPoints(myServer->segments);

    QString tobeSendMsg="/FEEDBACK_ANALYZE_Dissociative:";
    if(dissociativePoints.size()==0){
        qDebug()<<"no dissociative segs";
        tobeSendMsg += QString("server %1").arg(1);
    }else{
        qDebug()<<"dissociative seg exists";
        tobeSendMsg += QString("server %1").arg(0);
        tobeSendMsg +=",";
        for(auto it=dissociativePoints.begin(); it!=dissociativePoints.end(); it++){
            NeuronSWC s;
            stringToXYZ(*it, s.x, s.y, s.z);
            tobeSendMsg += QString("%1 %2 %3 %4 %5 %6").arg(200).arg(20).arg(0).arg(s.x).arg(s.y).arg(s.z);
            tobeSendMsg += ",";
        }
        tobeSendMsg.chop(1);
    }
    sendmsgs({tobeSendMsg});
}

void CollClient::analyzeAngles(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("analyzeAngles: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    if(!myServer->isSomaExists){
        qDebug()<<"soma not detected!";
        QString tobeSendMsg="/FEEDBACK_ANALYZE_Angle:";
        tobeSendMsg += QString("server %1").arg(-1);
        sendmsgs({tobeSendMsg});
        return;
    }
    else{
        set<string> angleErrPoints=getAngleErrPoints(8, myServer->somaCoordinate, myServer->segments);

        QString tobeSendMsg="/FEEDBACK_ANALYZE_Angle:";
        if(angleErrPoints.size()==0){
            qDebug()<<"no angle-error dendrite bifurcations";
            tobeSendMsg += QString("server %1").arg(1);
        }else{
            qDebug()<<"angle-error dendrite bifurcation exists";
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg +=",";
            for(auto it=angleErrPoints.begin(); it!=angleErrPoints.end(); it++){
                NeuronSWC s;
                stringToXYZ(*it, s.x, s.y, s.z);
                tobeSendMsg += QString("%1 %2 %3 %4 %5 %6").arg(200).arg(20).arg(0).arg(s.x).arg(s.y).arg(s.z);
                tobeSendMsg += ",";
            }
            tobeSendMsg.chop(1);
        }
        sendmsgs({tobeSendMsg});
    }
}

void CollClient::defineSoma(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("define soma: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    QString tobeSendMsg="/FEEDBACK_DEFINE_Soma:";
    QString info;

    if(!myServer->isSomaExists){
        qDebug()<<"soma not detected!";
        tobeSendMsg += QString("server %1").arg(0);
        tobeSendMsg += ",";
        info = "soma not detected!";
        tobeSendMsg += info;
        sendmsgs({tobeSendMsg});
        return;
    }
    else{
        myServer->mutex.lock();
        myServer->mutexForDetectOthers.lock();
        myServer->mutexForDetectMissing.lock();
        // 创建事件循环
        QEventLoop loop;

        // 连接信号和事件循环的退出槽
        connect(myServer->detectUtil, &CollDetection::removeErrorSegsDone, &loop, &QEventLoop::quit);

        emit detectUtilRemoveErrorSegs(true);

        // 等待事件循环退出
        loop.exec();
        myServer->mutexForDetectMissing.unlock();
        myServer->mutexForDetectOthers.unlock();
        myServer->mutex.unlock();
        QString fileSaveName = myServer->swcpath.left(myServer->swcpath.size()-QString(".ano.eswc").size())+"_somadefined.ano.eswc";
        bool result = setSomaPointRadius(fileSaveName, myServer->segments, myServer->somaCoordinate, 8, myServer->detectUtil, info);
        if(!result){
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg += ",";
            tobeSendMsg += info;
            sendmsgs({tobeSendMsg});
            return;
        }

        int number = getSomaNumberFromSwcFile(fileSaveName, 1.234, info);
        if(number == -2){
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg += ",";
            tobeSendMsg += info;
            sendmsgs({tobeSendMsg});
            return;
        }else if(number == -1){
            tobeSendMsg += QString("server %1").arg(0);
            tobeSendMsg += ",";
            info = "cannot find the \'n\' of soma point!";
            tobeSendMsg += info;
            sendmsgs({tobeSendMsg});
            return;
        }else{
            QList<NeuronSWC> neuron, result;
            neuron = readSWC_file(fileSaveName).listNeuron;
            if (!SortSWCSimplify(neuron, result, number, info))
            {
                tobeSendMsg += QString("server %1").arg(0);
                tobeSendMsg += ",";
                tobeSendMsg += info;
                sendmsgs({tobeSendMsg});
                return;
            }
            if (!export_list2file(result, fileSaveName, myServer->swcpath))
            {
                tobeSendMsg += QString("server %1").arg(0);
                tobeSendMsg += ",";
                info = "cannot open swc file!";
                tobeSendMsg += info;
                sendmsgs({tobeSendMsg});
                return;
            }
            tobeSendMsg += QString("server %1").arg(1);
            tobeSendMsg += ",";
            info = "success!";
            tobeSendMsg += info;
            sendmsgs({tobeSendMsg});
        }
    }
}

void CollClient::getSomaPos(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("get soma pos: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    QString tobeSendMsg="/FEEDBACK_SEND_SomaPos:";
    QString info;

    QMutexLocker locker(&myServer->mutex);
    if(myServer->markers.empty())
    {
        tobeSendMsg += QString("server %1").arg(0);
        tobeSendMsg += ",";
        info="no marker!";
        tobeSendMsg += info;
        sendmsgs({tobeSendMsg});
    }
    else{
        myServer->somaCoordinate.x = myServer->markers.last().x;
        myServer->somaCoordinate.y = myServer->markers.last().y;
        myServer->somaCoordinate.z = myServer->markers.last().z;
        myServer->isSomaExists = true;
        myServer->markers.move(myServer->markers.size() - 1, 0);
        emit serverImediateSave(false);

        tobeSendMsg += QString("server %1").arg(1);
        tobeSendMsg += ",";
        info="success";
        tobeSendMsg += info;
        sendmsgs({tobeSendMsg});
    }
}

bool CollClient::connectToDBMS(){
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
            qDebug()<<"Login Failed!" + QString::fromStdString(response.metainfo().message());
            return false;
        }

    }else{
        qDebug()<<"Error" + QString::fromStdString(status.error_message());
    }
    return false;
}
