#include "coll_server.h"
#include <unistd.h>
#include "utils.h"
#include <thread>
#include <QThread>
#include <chrono>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
extern QFile *logfile;

CollServer* CollServer::curServer=nullptr;

QStringList CollServer::msglist;
int CollServer::processedmsgcnt=0;
int CollServer::savedmsgcnt=0;
int CollServer::receivedcnt=0;
QMap<QString,CollClient*> CollServer::hashmap;
V_NeuronSWC_list CollServer::segments;
XYZ CollServer::somaCoordinate;
bool CollServer::isSomaExists;
QList<CellAPO> CollServer::markers;
//int receivedcnt;
QString CollServer::swcpath;
QString CollServer::apopath;
QString CollServer::anopath;

CollServer::CollServer(QString port,QString image,QString neuron,QString anoname,QString prefix,QObject *parent)
    :QTcpServer(parent),Port(port),Image(image),Neuron(neuron),AnoName(anoname),Prefix(prefix+"/data/"+image+"/"+neuron+"/"+anoname)
    ,timerForAutoSave(new QTimer(this)),timerForDetection(new QTimer(this)),timerForAutoExit(new QTimer(this))
{
    qDebug()<<"MainThread:"<<QThread::currentThreadId();
    curServer=this;
    accessManager=new QNetworkAccessManager(this);
    HostAddress=="http://114.117.165.134:26000/SuperUser";

    qRegisterMetaType<qintptr>("qintptr");

    auto nt=readSWC_file(Prefix+"/"+AnoName+".ano.eswc");
    segments=NeuronTree__2__V_NeuronSWC_list(nt);
    markers=readAPO_file(Prefix+"/"+AnoName+".ano.apo");
    swcpath=Prefix+"/"+AnoName+".ano.eswc";
    apopath=Prefix+"/"+AnoName+".ano.apo";
    anopath=Prefix+"/"+AnoName+".ano";
    somaCoordinate=getSomaCoordinate(apopath);
    // 3分钟执行一次
    timerForAutoSave->start(3*60*1000);
    timerForDetection->start(1*60*1000);
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
    connect(timerForAutoSave,&QTimer::timeout,this,&CollServer::autoSave);
    connect(timerForDetection,&QTimer::timeout,this,&CollServer::autoDetect);
    connect(timerForAutoExit,&QTimer::timeout,this,&CollServer::autoExit);
    connect(&CollClient::timerforupdatemsg,&QTimer::timeout,[]{
        for (auto iter=hashmap.begin();iter!=hashmap.end();iter++){
            qDebug()<<"user:"<<iter.key()<<" state:"<<iter.value()->state();
        }
//        auto sockets=hashmap.values();
//        for(auto &socket:sockets){
//            socket->sendmsgs2client(10);
//        }
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
    accessManager->deleteLater();

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
    savedmsgcnt=processedmsgcnt;
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
        auto sockets=hashmap.values();
//        for(auto &socket:sockets){
//            socket->updatesendmsgcnt2processed();
//        }
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

XYZ CollServer::getSomaCoordinate(QString apoPath){
    isSomaExists=false;
    XYZ coordinate;
    coordinate.x=-1;
    coordinate.y=-1;
    coordinate.z=-1;
    QFile qf(apoPath);
    if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"apofile open error";
        return coordinate;
    }
    char *buf;
    char _buf[1000];
    qf.readLine(_buf, sizeof(_buf));
    for (buf=_buf; (*buf && *buf==' '); buf++);
    if (buf[0]=='#' ||buf[0]=='\0')
    {
        if(!qf.atEnd())
        {
            qf.readLine(_buf, sizeof(_buf));
            for (buf=_buf; (*buf && *buf==' '); buf++);
        }
        else{
            qDebug()<<"apofile format error";
            return coordinate;
        }
    }
    else{
        qDebug()<<"apofile format error";
        return coordinate;
    }
    QStringList qsl = QString(buf).split(",");
    if (qsl.size()==0){
        qDebug()<<"apofile format error";
        return coordinate;
    }
    else{
        for (int i=4; i<qsl.size(); i++)
        {
            qsl[i].truncate(200); //change from 99 to 200, 20121212, by PHC
            if (i==4) coordinate.z = qsl[i].toFloat();
            if (i==5) coordinate.x = qsl[i].toFloat();
            if (i==6)
            {
                coordinate.y = qsl[i].toFloat();
                isSomaExists=true;
                break;
            }
        }
    }
    return coordinate;
}

vector<NeuronSWC> CollServer::specStructsDetection(V_NeuronSWC_list inputSegList, double dist_thresh){

    vector<NeuronSWC> outputErroneousPoints;
    outputErroneousPoints.clear();

    map<string, set<size_t> > wholeGrid2segIDmap;
    map<string, bool> isEndPointMap;
    map<string, set<string>> parentMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2segIDmap[gridKey].insert(size_t(i));

            if(seg.row[j].parent!=-1){
                float x2Label=seg.row[seg.row[j].parent].x;
                float y2Label=seg.row[seg.row[j].parent].y;
                float z2Label=seg.row[seg.row[j].parent].z;
                QString parentKeyQ=QString::number(x2Label) + "_" + QString::number(y2Label) + "_" + QString::number(z2Label);
                string parentKey=parentKeyQ.toStdString();
                parentMap[gridKey].insert(parentKey);
            }

            if(j == 0 || j == seg.row.size() - 1){
                isEndPointMap[gridKey] = true;
            }
        }
    }

    for(map<string, set<size_t> >::iterator it = wholeGrid2segIDmap.begin(); it != wholeGrid2segIDmap.end(); ++it){
        if(it->second.size() > 5){
            qDebug()<<it->first.c_str()<<" "<<it->second.size();
        }

    }
    qDebug()<<"whole end";

    vector<string> points;
    vector<set<int>> linksIndex;
    vector<vector<int>> linksIndexVec;
    map<string,int> pointsIndexMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(j==0 || j==seg.row.size()-1){
                //在pointsIndexMap中找不到某个线的末端点
                if(pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }else{
                if(wholeGrid2segIDmap[gridKey].size()>1 &&
                    isEndPointMap.find(gridKey) != isEndPointMap.end() &&
                    pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }
        }
    }
    qDebug()<<"points size: "<<points.size();

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
        vector<int> segIndexs;
        set<int> segIndexsSet;
        segIndexs.clear();
        segIndexsSet.clear();
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(pointsIndexMap.find(gridKey) != pointsIndexMap.end()){
                int index = pointsIndexMap[gridKey];
                if(segIndexsSet.find(index) == segIndexsSet.end()){
                    segIndexs.push_back(index);
                    segIndexsSet.insert(pointsIndexMap[gridKey]);
                }
            }
        }
        //        qDebug()<<"i : "<<i<<"seg size: "<<seg.row.size()<<" segIndexsSize: "<<segIndexs.size();
        for(size_t j=0; j<segIndexs.size()-1; ++j){
            if(segIndexs[j] == 1 || segIndexs[j+1] == 1){
                qDebug()<<segIndexs[j]<<" "<<segIndexs[j+1];
            }
            linksIndex[segIndexs[j]].insert(segIndexs[j+1]);
            linksIndexVec[segIndexs[j]].push_back(segIndexs[j+1]);
            linksIndex[segIndexs[j+1]].insert(segIndexs[j]);
            linksIndexVec[segIndexs[j+1]].push_back(segIndexs[j]);
        }
    }

    qDebug()<<"link map end";

    for(size_t i=0; i<points.size(); ++i){
        if(linksIndex[i].size() > 3){
            qDebug()<<i<<" link size: "<<linksIndex[i].size();
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            s.type = 8;
            outputErroneousPoints.push_back(s);
        }
    }

    vector<int> counts;
    for(int i=0;i<linksIndexVec.size();i++){
        counts.push_back(linksIndexVec[i].size());
    }

    qDebug()<<"outputError size:"<<outputErroneousPoints.size();

    //    map<string, set<size_t> > wholeGrid2segIDmap2;
    //    for(auto it : wholeGrid2segIDmap){
    //        wholeGrid2segIDmap2[it.first]=it.second;
    //    }
    vector<vector<int>> newLinksIndexVec(linksIndexVec);

    bool isDeleteEnd = false;
    while(!isDeleteEnd){
        isDeleteEnd = true;
        for(int i=0; i<points.size(); ++i){
            if(newLinksIndexVec[i].size() == 1){
                int linkIndex = *(newLinksIndexVec[i].begin());
                newLinksIndexVec[i].clear();
                //                linksIndex[linkIndex].erase(std::find(linksIndex[linkIndex].begin(),linksIndex[linkIndex].end(),i));
                //                linksIndexVec[linkIndex].erase(i);
                for (auto iter = newLinksIndexVec[linkIndex].begin(); iter != newLinksIndexVec[linkIndex].end(); iter++)
                {
                    if(*iter==i){
                        newLinksIndexVec[linkIndex].erase(iter);
                        break;
                    }
                }

                isDeleteEnd = false;
            }
        }
    }


    qDebug()<<"loop end";

    vector<string> newpoints;

    for(size_t i=0; i<points.size(); ++i){
        if(newLinksIndexVec[i].size()>=2)
            newpoints.push_back(points[i]);
    }

    size_t start=0;
    for(size_t i=0; i<newpoints.size(); ++i){
        qDebug()<<QString::fromStdString(newpoints[i])<<" "<<parentMap[newpoints[i]].size();
        /*if(newLinksIndexVec[i].size()>=2&&counts[i]>=3&&newLinksIndexVec[i].size()!=counts[i])*/
        if(parentMap[newpoints[i]].size()>=2){
            size_t interval=i-start;
            int nums=interval/12;
            for(int j=0;j<nums;j++){
                NeuronSWC s;
                stringToXYZ(newpoints[start+(j+1)*12],s.x,s.y,s.z);
                s.type = 0;
                outputErroneousPoints.push_back(s);
            }

            NeuronSWC s;
            stringToXYZ(newpoints[i],s.x,s.y,s.z);
            s.type = 0;
            outputErroneousPoints.push_back(s);

            start=i+1;
            qDebug()<<"loop exists";
        }

    }

    if(start<newpoints.size()){
        size_t interval=newpoints.size()-1-start;
        int nums=interval/12;
        for(int j=0;j<nums;j++){
            NeuronSWC s;
            stringToXYZ(newpoints[start+(j+1)*12],s.x,s.y,s.z);
            s.type = 0;
            outputErroneousPoints.push_back(s);
        }
    }

    vector<vector<size_t>> pairs;
    set<size_t> pset;

    size_t pre_tip_id=-1;
    size_t cur_tip_id=-1;

    double soma_radius=30;
    for(size_t i=0; i<points.size(); i++){
        if(linksIndex[i].size() == 3){
            pre_tip_id=cur_tip_id;
            cur_tip_id=i;
            if(pre_tip_id!=-1){
                NeuronSWC n1;
                stringToXYZ(points[pre_tip_id],n1.x,n1.y,n1.z);
                n1.type=6;
                NeuronSWC n2;
                stringToXYZ(points[cur_tip_id],n2.x,n2.y,n2.z);
                n2.type=6;
                if(isSomaExists){
                    if(distance(n1.x,somaCoordinate.x,n1.y,somaCoordinate.y,n1.z,somaCoordinate.z)>soma_radius
                        &&distance(n2.x,somaCoordinate.x,n2.y,somaCoordinate.y,n2.z,somaCoordinate.z)>soma_radius){
                        double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                        if(distance((n1.x+n2.x)/2,somaCoordinate.x,(n1.y+n2.y)/2,somaCoordinate.y,(n1.z+n2.z)/2,somaCoordinate.z)>1e-7&&dist<dist_thresh){
                            vector<size_t> v={pre_tip_id,cur_tip_id};
                            pairs.push_back(v);
                            pset.insert(pre_tip_id);
                            pset.insert(cur_tip_id);
                        }
                    }
                }
                else{
                    double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                    if(dist<dist_thresh){
                        vector<size_t> v={pre_tip_id,cur_tip_id};
                        pairs.push_back(v);
                        pset.insert(pre_tip_id);
                        pset.insert(cur_tip_id);
                    }
                }

            }
        }
    }

    qDebug()<<pairs;
    //    qDebug()<<points;

    for(auto it=pset.begin(); it!=pset.end(); it++){
        qDebug()<<*it;
        NeuronSWC n;
        stringToXYZ(points[*it],n.x,n.y,n.z);
        n.type=6;
        outputErroneousPoints.push_back(n);
    }

    return outputErroneousPoints;

}

void CollServer::handleMulFurcation(vector<NeuronSWC>& outputErroneousPoints, int& count){
    for(int i=0;i<outputErroneousPoints.size();i++){
        if(isSomaExists)
        {
            if(outputErroneousPoints[i].type == 8
                && (abs(outputErroneousPoints[i].x - somaCoordinate.x) > 5 ||
                    abs(outputErroneousPoints[i].y - somaCoordinate.y) > 5  ||
                    abs(outputErroneousPoints[i].z - somaCoordinate.z) > 5 ))
            {
                count++;
                QStringList result;
                result.push_back(QString("%1 server").arg(0));
                result.push_back(QString("%1 %2 %3 %4").arg(outputErroneousPoints[i].type).arg(outputErroneousPoints[i].x).arg(outputErroneousPoints[i].y).arg(outputErroneousPoints[i].z));
                QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
                //            const std::string data=msg.toStdString();
                //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
                auto sockets=hashmap.values();
                emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
                qDebug()<<"Server finish /WARN_MulBifurcation";

                //                for(auto &socket:sockets){
                //                    socket->sendmsgs({msg});
                //                }
                emit clientSendMsgs({msg});
            }
        }

        else{
            if(outputErroneousPoints[i].type == 8)
            {
                count++;
                QStringList result;
                result.push_back(QString("%1 server").arg(0));
                result.push_back(QString("%1 %2 %3 %4").arg(outputErroneousPoints[i].type).arg(outputErroneousPoints[i].x).arg(outputErroneousPoints[i].y).arg(outputErroneousPoints[i].z));
                QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
                //            const std::string data=msg.toStdString();
                //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
                //                auto sockets=hashmap.values();
                emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
                qDebug()<<"Server finish /WARN_MulBifurcation";

                //                for(auto &socket:sockets){
                //                    socket->sendmsgs({msg});
                //                }
                emit clientSendMsgs({msg});
            }
        }
    }
}

void CollServer::handleLoop(vector<NeuronSWC>& outputErroneousPoints, int& count){
    for(int i=0;i<outputErroneousPoints.size();i++){
        if(outputErroneousPoints[i].type == 0){
            count++;
            QStringList result;
            result.push_back(QString("%1 server").arg(0));
            result.push_back(QString("%1 %2 %3 %4").arg(outputErroneousPoints[i].type).arg(outputErroneousPoints[i].x).arg(outputErroneousPoints[i].y).arg(outputErroneousPoints[i].z));
            QString msg=QString("/WARN_Loop:"+result.join(","));
            //            auto sockets=hashmap.values();
            emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_Loop:").size()));
            qDebug()<<"Server finish /WARN_Loop";

            //            for(auto &socket:sockets){
            //                socket->sendmsgs({msg});
            //            }
            emit clientSendMsgs({msg});
        }
    }
}

void CollServer::handleCrossing(vector<NeuronSWC>& crossingPoints, int& count){
    count+=crossingPoints.size();
    for(int i=0;i<crossingPoints.size();i++){
        QStringList result;
        result.push_back(QString("%1 server").arg(0));
        result.push_back(QString("%1 %2 %3 %4").arg(crossingPoints[i].type).arg(crossingPoints[i].x).arg(crossingPoints[i].y).arg(crossingPoints[i].z));
        QString msg=QString("/WARN_Crossing:"+result.join(","));
        //            const std::string data=msg.toStdString();
        //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
        //            auto sockets=hashmap.values();
        emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_Crossing:").size()));
        qDebug()<<"Server finish /WARN_Crossing";
        //                for(auto &socket:sockets){
        //                    socket->sendmsgs({msg});
        //                }
        emit clientSendMsgs({msg});
    }

    if(crossingPoints.size()!=0){
        QNetworkRequest request;
        request.setUrl(QUrl(HostAddress+"/detect/crossing"));
        qDebug()<<HostAddress;
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        QJsonObject json;
        QString emp="123";
        json.insert("exmple", emp);

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray = str.toUtf8();

        qDebug()<<byteArray;
        QNetworkReply* reply = accessManager->post(request, byteArray);
        if(!reply)
            qDebug()<<"reply = nullptr";

        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"handleCrossing"<<code;
        if(code==200)
        {

        }
        else
        {

        }
    }

}

void CollServer::handleTip(vector<NeuronSWC>& tipPoints, int& count){
    //TODO
    count+=tipPoints.size();
    for(int i=0;i<tipPoints.size();i++){
        QStringList result;
        result.push_back(QString("%1 server").arg(0));
        result.push_back(QString("%1 %2 %3 %4").arg(tipPoints[i].type).arg(tipPoints[i].x).arg(tipPoints[i].y).arg(tipPoints[i].z));
        QString msg=QString("/WARN_Tip:"+result.join(","));
        //            const std::string data=msg.toStdString();
        //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
        //            auto sockets=hashmap.values();
        emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_Tip:").size()));
        qDebug()<<"Server finish /WARN_Tip";
        //                for(auto &socket:sockets){
        //                    socket->sendmsgs({msg});
        //                }
        emit clientSendMsgs({msg});
    }

    if(tipPoints.size()!=0){
        QNetworkRequest request;
        request.setUrl(QUrl(HostAddress+"/detect/missing"));
        qDebug()<<HostAddress;
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        QJsonObject json;
        QString emp="123";
        json.insert("exmple", emp);

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray = str.toUtf8();

        qDebug()<<byteArray;
        QNetworkReply* reply = accessManager->post(request, byteArray);
        if(!reply)
            qDebug()<<"reply = nullptr";

        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"handleCrossing"<<code;
        if(code==200)
        {

        }
        else
        {

        }
    }
}


void CollServer::autoDetectSpecStructs(){
    int count=0;
    vector<NeuronSWC> outputErroneousPoints = specStructsDetection(segments);
    vector<NeuronSWC> crossingPoints;
    for(int i=0;i<outputErroneousPoints.size();i++){
        if(outputErroneousPoints[i].type == 6){
            crossingPoints.push_back(outputErroneousPoints[i]);
        }
    }

    handleMulFurcation(outputErroneousPoints, count);
    handleLoop(outputErroneousPoints, count);
    handleCrossing(crossingPoints, count);

    if(count!=0){
        imediateSave();
    }
}

void CollServer::autoDetect(){
    autoDetectSpecStructs();
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


