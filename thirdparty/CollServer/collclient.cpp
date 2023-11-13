#include "collclient.h"
#include "coll_server.h"
#include "utils.h"
#include <cmath>
#include <analyze.h>
//#include <algorithm>
extern QFile* logfile;


QTimer CollClient::timerforupdatemsg;

CollClient:: CollClient(qintptr handle, CollServer* curServer, QObject *parent):QTcpSocket(parent){
    setSocketDescriptor(handle);
    myServer=curServer;

//    如果一分钟内没有登陆好，则断开连接
    QTimer::singleShot(60*1000,this,[this]{
        if(username==""){
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

    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,1);
    auto segs=NeuronTree__2__V_NeuronSWC_list(addnt).seg;
//    segs[0].printInfo();

//    bool flag;
//    if(fabs(point1.x-segs[0].row[0].x)<1e-4&&fabs(point1.y-segs[0].row[0].y)<1e-4&&fabs(point1.z-segs[0].row[0].z)<1e-4)
//        flag=true;
//    else
//        flag=false;

    myServer->mutex.lock();
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
        set<size_t> segIds1;
        set<size_t> segIds2;
        int firstIndex = -1;
        int firstEndIndex = -1;
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
                float xLabel = it->row[index].x;
                float yLabel = it->row[index].y;
                float zLabel = it->row[index].z;
                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
                string gridKey = gridKeyQ.toStdString();
                map<string, set<size_t>> wholeGrid2SegIDMap = getWholeGrid2SegIDMap(myServer->segments);
                segIds1 = wholeGrid2SegIDMap[gridKey];
                firstIndex = index;
                firstEndIndex = it->row.size()-1;
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
                float xLabel = it->row[index].x;
                float yLabel = it->row[index].y;
                float zLabel = it->row[index].z;
                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
                string gridKey = gridKeyQ.toStdString();
                map<string, set<size_t>> wholeGrid2SegIDMap = getWholeGrid2SegIDMap(myServer->segments);
                segIds2 = wholeGrid2SegIDMap[gridKey];
            }
        }
        else
        {
            std::cerr<<"INFO:not find connected seg ,"<<msg.toStdString()<<std::endl;
        }

        if(segIds1.size()==1 && segIds2.size()==1 && firstIndex==firstEndIndex && firstEndIndex!=-1 )
            isNeedReverse = true;
        if(segIds1.size()==1 && segIds2.size()>1)
            isNeedReverse = true;
        if(isNeedReverse)
            reverseSeg(segs[0]);
    }

    myServer->segments.append(segs[0]);

    myServer->mutexForDetectOthers.lock();
    myServer->last1MinSegments.append(segs[0]);
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    myServer->last3MinSegments.append(segs[0]);
    myServer->mutexForDetectMissing.unlock();

    myServer->mutex.unlock();

    qDebug()<<"server addseg";
//    for(int i=0;i<myServer->segments.seg.size();i++){
//        myServer->segments.seg[i].printInfo();
//    }
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
    auto delnt=convertMsg2NT(pointlist,clienttype,useridx,isMany);
    auto delsegs=NeuronTree__2__V_NeuronSWC_list(delnt).seg;

    int count=0;
    myServer->mutex.lock();
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),delsegs[i]);
        if(it!=myServer->segments.seg.end())
        {
            myServer->segments.seg.erase(it);
            if(count<5)
                qDebug()<<"server delseg";
            count++;
        }
        else
            std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;

    }

    myServer->mutexForDetectOthers.lock();
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->last1MinSegments.seg.begin(),myServer->last1MinSegments.seg.end(),delsegs[i]);
        if(it!=myServer->last1MinSegments.seg.end())
        {
            myServer->last1MinSegments.seg.erase(it);
            qDebug()<<"server delseg in latestSegments";
        }
    }
    myServer->mutexForDetectOthers.unlock();

    myServer->mutexForDetectMissing.lock();
    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->last3MinSegments.seg.begin(),myServer->last3MinSegments.seg.end(),delsegs[i]);
        if(it!=myServer->last3MinSegments.seg.end())
        {
            myServer->last3MinSegments.seg.erase(it);
            qDebug()<<"server delseg in latestSegments";
        }
    }
    myServer->mutexForDetectMissing.unlock();
    myServer->mutex.unlock();

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

    auto segnt=convertMsg2NT(pointlist,clienttype,useridx,1);
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

    if (myServer->segments.seg[segInfo[0].segID].to_be_deleted)
    {
        qDebug()<<"enter tracedNeuron.seg[segInfo[0]]";
        vector<V_NeuronSWC> connectedSegDecomposed = decompose_V_NeuronSWC(myServer->segments.seg[segInfo[1].segID]);
        for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
            myServer->segments.seg.push_back(*addedIt);

        myServer->segments.seg[segInfo[1].segID].to_be_deleted = true;
        myServer->segments.seg[segInfo[1].segID].on = false;

    }
    else if (myServer->segments.seg[segInfo[1].segID].to_be_deleted)
    {
        qDebug()<<"enter tracedNeuron.seg[segInfo[1]]";
        vector<V_NeuronSWC> connectedSegDecomposed = decompose_V_NeuronSWC(myServer->segments.seg[segInfo[0].segID]);
        for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
            myServer->segments.seg.push_back(*addedIt);

        myServer->segments.seg[segInfo[0].segID].to_be_deleted = true;
        myServer->segments.seg[segInfo[0].segID].on = false;
    }

    std::vector<V_NeuronSWC>::iterator iter = myServer->segments.seg.begin();
    while (iter != myServer->segments.seg.end())
        if (iter->to_be_deleted)
            iter = myServer->segments.seg.erase(iter);
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

    auto tempnt=convertMsg2NT(pointlist,clienttype,useridx,1);
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
        myServer->segments.seg.erase(it);
    }

    else
    {
        std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
        return;
    }

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

    auto retypent=convertMsg2NT(pointlist,clienttype,useridx,isMany);
    auto retypesegs=NeuronTree__2__V_NeuronSWC_list(retypent).seg;

    int count=0;

    QMutexLocker locker(&myServer->mutex);
    for(int i=0;i<retypesegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),retypesegs[i]);
        if(it==myServer->segments.seg.end()){
            std::cerr<<"INFO:not find retype seg ,"<<msg.toStdString()<<std::endl;
            //            myServer->mutex.unlock();
            return;
        }
        int now=QDateTime::currentMSecsSinceEpoch();
        for(auto &unit:it->row){
            unit.type=newcolor;
            unit.level=now-unit.timestamp;
            unit.creatmode=useridx*10+clienttype;
        }
        if(count<5)
            qDebug()<<"server retypesegment";
        count++;
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

    QMutexLocker locker(&myServer->mutex);
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=4) continue;
        marker.color.r=neuron_type_color[markerinfo[0].toUInt()][0];
        marker.color.g=neuron_type_color[markerinfo[0].toUInt()][1];
        marker.color.b=neuron_type_color[markerinfo[0].toUInt()][2];
        marker.x=markerinfo[1].toDouble();
        marker.y=markerinfo[2].toDouble();
        marker.z=markerinfo[3].toDouble();

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
        if(markerinfo.size()!=4) continue;
        marker.color.r=neuron_type_color[markerinfo[0].toUInt()][0];
        marker.color.g=neuron_type_color[markerinfo[0].toUInt()][1];
        marker.color.b=neuron_type_color[markerinfo[0].toUInt()][2];
        marker.x=markerinfo[1].toDouble();
        marker.y=markerinfo[2].toDouble();
        marker.z=markerinfo[3].toDouble();
        if(myServer->isSomaExists&&sqrt((marker.x-myServer->somaCoordinate.x)*(marker.x-myServer->somaCoordinate.x)+
                (marker.y-myServer->somaCoordinate.y)*(marker.y-myServer->somaCoordinate.y)+
                (marker.z-myServer->somaCoordinate.z)*(marker.z-myServer->somaCoordinate.z))<1)
        {
            qDebug()<<"cannot delete the soma marker";
//            myServer->mutex.unlock();
            return;
        }
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
        qDebug()<<"error: send msg to "<<this->username<<",but connect is "<<this->state();
//        ondisconnect();
        return;
    }
    qDebug()<<"server send msgs";

    const std::string data=msgs.join(';').toStdString();
    const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
    // QString::fromStdString(header)将header转换为utf-8编码的字符串
    qDebug()<<"write to "<<username<<",headsize = "<<header.size()<<"，sendsize = "<<write(header.c_str(),header.size())<<","<<QString::fromStdString(header);
    qDebug()<<"write to "<<username<<",datasize = "<<data.size()<<"，sendsize = "<<write(data.c_str(),data.size())<<","<<QString::fromStdString(data);
    this->flush();
}

void CollClient::sendfiles(const QStringList &filepaths)
{
    if(this->state()!=QAbstractSocket::ConnectedState)
        return;

    for(const auto &path:filepaths){
        QString filename=QFileInfo(path).fileName();
        QFile f(path);
        if(f.open(QIODevice::ReadOnly)){
            QByteArray data=f.readAll();
            const std::string header=QString("DataTypeWithSize:%1 %2 %3\n").arg(1).arg(filename.size()).arg(data.size()).toStdString();
            write(header.c_str(),header.size());
            write(filename.toStdString().c_str(),filename.size());
            write(data,data.size());
        }
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
            if (ps.size()>3){
                std::cerr<<"login in error:"<<msg.toStdString();
//                this->disconnectFromHost();
//                this->close();//关闭读
                emit exitNow();
                return;
            }
            qDebug()<<"subThread:"<<QThread::currentThreadId();
            receiveuser(ps[0], ps[1]);
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
                        std::cerr<<username.toStdString()+" receive not match format\n";
                        emit exitNow();
                    }

                    auto ps=msg.right(msg.size()-QString("DataTypeWithSize:").size()).split(' ');
                    if (ps[0].toInt()!=0){
                        this->write("Socket Receive ERROR!");
                        std::cerr<<username.toStdString()+" receive not match format\n";
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
                    log=QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ") + QString::number(myServer->receivedcnt) + " receive from " + username + ":" + QString(data);

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
    this->close();//关闭读
    if(myServer->hashmap.contains(username)&&myServer->hashmap[username]==this)
        myServer->hashmap.remove(username);
    if(myServer->hashmap.size()==0)
    {
        emit noUsers();
    }
    updateuserlist();
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

void CollClient::receiveuser(const QString user, QString RES)
{
    username=user;
    myServer->mutex.lock();
    if(myServer->hashmap.contains(user))
    {
        std::cerr<<"ERROR:"+user.toStdString()+" is duolicate,will remove the first\n";
        emit myServer->clientDisconnectFromHost(myServer->hashmap[user]);
    }
    myServer->hashmap[user]=this;
    myServer->RES=RES;
    myServer->mutex.unlock();
    updateuserlist();

    myServer->imediateSave();
    //todo发送保存的文件
    sendfiles({
    myServer->anopath,myServer->apopath,myServer->swcpath
              });

    myServer->mutex.lock();
    sendmsgcnt=myServer->savedmsgcnt;
    qDebug()<<"receive user init sendmsgcnt = "<<sendmsgcnt;
    myServer->mutex.unlock();
    // 获取协同的ano文件名
    QString msg=QString("STARTCOLLABORATE:%1").arg(myServer->anopath.section('/',-1,-1));
    sendmsgs({msg});

    if(!myServer->getTimerForDetectOthers()->isActive())
        emit serverStartTimerForDetectOthers();
    if(!myServer->getTimerForDetectLoops()->isActive())
        emit serverStartTimerForDetectLoops();
    if(!myServer->getTimerForDetectTip()->isActive())
        emit serverStartTimerForDetectTip();
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
    qDebug()<<"send to "<< this->username<<" :("<<myServer->msglist.begin()+this->sendmsgcnt
           <<","<<myServer->msglist.begin()+end<<")/"<<myServer->msglist.size();
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

    //////////////////////////////////////////// HEAD TAIL CONNECTION ////////////////////////////////////////////
    if ((segInfo.at(0).head_tail == -1 || segInfo.at(0).head_tail == 2) && (segInfo.at(1).head_tail == -1 || segInfo.at(1).head_tail == 2))
    {
        segInfoUnit mainSeg, branchSeg;
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
    }
    //////////////////////////////////////////// END of [HEAD TAIL CONNECTION] ////////////////////////////////////////////

    //////////////////////////////////////////// BRANCHING CONNECTION ////////////////////////////////////////////
    if ((segInfo.at(0).head_tail != -1 && segInfo.at(0).head_tail != 2) ^ (segInfo.at(1).head_tail != -1 && segInfo.at(1).head_tail != 2))
    {
        segInfoUnit mainSeg, branchSeg;
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
    }
    //////////////////////////////////////////// END of [BRANCHING CONNECTION] ////////////////////////////////////////////

    return;
}

void CollClient::analyzeSomaNearBy(const QString msg){
    QStringList headerlist=msg.split(' ',Qt::SkipEmptyParts);
    int clienttype=headerlist[0].toUInt();
    int useridx=headerlist[1].toUInt();
    qDebug()<<QString("analyzeSomaNearBy: clienttype=%1, useridx=%2").arg(clienttype).arg(useridx);

    if(!myServer->isSomaExists){
        qDebug()<<"soma not detected!";
        return;
    }
    else{
        myServer->imediateSave();
        vector<int> counts=getMulfurcationsCountNearSoma(12, myServer->somaCoordinate, myServer->segments);
        QString tobeSendMsg="/ANALYZE_SomaNearBy:";
        if((counts[0] + counts[1])!=1){
            qDebug()<<"the soma is not connected to one point!";
            tobeSendMsg += QString("server %1").arg(0);
        }else{
            qDebug()<<"the soma has been connected to one point";
            tobeSendMsg += QString("server %1").arg(1);
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
        return;
    }
    else{
        myServer->imediateSave();
        map<string,set<int>> specPointsMap=getColorChangedPoints(myServer->segments);
        set<string> resultSet;
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
                         coor.z, myServer->somaCoordinate.z)<12)
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
                             coor.z, myServer->somaCoordinate.z)>12)
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
                         s.z, myServer->somaCoordinate.z)<12)
            {
                it=resultSet.erase(it);
            }else{
                it++;
            }
        }

        QString tobeSendMsg="/ANALYZE_ColorMutation:";
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
                tobeSendMsg += QString("%1 %2 %3 %4").arg(2).arg(s.x).arg(s.y).arg(s.z);
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

    myServer->imediateSave();
    set<string> dissociativePoints=getDissociativeSegEndPoints(myServer->segments);

    QString tobeSendMsg="/ANALYZE_Dissociative:";
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
            tobeSendMsg += QString("%1 %2 %3 %4").arg(2).arg(s.x).arg(s.y).arg(s.z);
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
        return;
    }
    else{
        myServer->imediateSave();
        set<string> angleErrPoints=getAngleErrPoints(12, myServer->somaCoordinate, myServer->segments);

        QString tobeSendMsg="/ANALYZE_Angle:";
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
                tobeSendMsg += QString("%1 %2 %3 %4").arg(2).arg(s.x).arg(s.y).arg(s.z);
                tobeSendMsg += ",";
            }
            tobeSendMsg.chop(1);
        }
        sendmsgs({tobeSendMsg});
    }
}
