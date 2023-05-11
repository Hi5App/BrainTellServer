#include "collclient.h"
#include "coll_server.h"
#include "utils.h"
#include <cmath>
//#include <algorithm>
extern QFile* logfile;


QTimer CollClient::timerforupdatemsg;
//QStringList CollClient::msglist;
//int CollClient::processedmsgcnt=0;
//int CollClient::savedmsgcnt=0;
//int CollClient::receivedcnt=0;
//QMap<QString,CollClient*> CollClient::hashmap;
//V_NeuronSWC_list CollClient::segments;
//XYZ CollClient::somaCoordinate;
//bool CollClient::isSomaExists;
//QList<CellAPO> CollClient::markers;
//int receivedcnt;
//QString CollClient::swcpath;
//QString CollClient::apopath;
//QString CollClient::anopath;

CollClient:: CollClient(qintptr handle, CollServer* curServer, QObject *parent):QTcpSocket(parent){
    setSocketDescriptor(handle);
    myServer=curServer;
//    connect(this,&QTcpSocket::readyRead,this,&CollClient::onread);
//    connect(this,&QTcpSocket::disconnected,this,&CollClient::ondisconnect);
//    connect(this,&QAbstractSocket::errorOccurred, this, &CollClient::onError);
//    connect(this,&CollClient::noUsers,myServer,&CollServer::imediateSave);
//    connect(this,&CollClient::removeList,myServer,&CollServer::RemoveList);
//    如果一分钟内没有登陆好，则断开连接
    QTimer::singleShot(60*1000,this,[this]{
        if(username==""){
//            this->disconnectFromHost();
//            this->close();//关闭读
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
    // 其实是username
    int useridx=headerlist[1].toUInt();

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }
    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,0);
    myServer->segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
    qDebug()<<"server addseg";
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
    }
    auto delnt=convertMsg2NT(pointlist,clienttype,useridx,isMany);
    auto delsegs=NeuronTree__2__V_NeuronSWC_list(delnt).seg;

    for(int i=0;i<delsegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),delsegs[i]);
        if(it!=myServer->segments.seg.end())
        {
            myServer->segments.seg.erase(it);
            qDebug()<<"server delseg";
        }
        else
            std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
    }
}

void CollClient::connectseg(const QString msg){
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
    auto addnt=convertMsg2NT(pointlist,clienttype,useridx,0);
    myServer->segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
    qDebug()<<"server connectseg";
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

        for(auto it=myServer->markers.begin();it!=myServer->markers.end(); ++it)
        {
            if(it->color.r==marker.color.r&&it->color.g==marker.color.g&&it->color.b==marker.color.b
                &&abs(it->x-marker.x)<1&&abs(it->y-marker.y)<1&&abs(it->z-marker.z)<1)
            {
                qDebug()<<"the marker has already existed";
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
    }
    CellAPO marker;
    int idx=-1;
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
    }
    CellAPO marker;
    int idx=-1;
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
    }

    auto retypent=convertMsg2NT(pointlist,clienttype,useridx,isMany);
    auto retypesegs=NeuronTree__2__V_NeuronSWC_list(retypent).seg;

    for(int i=0;i<retypesegs.size();i++){
        auto it=findseg(myServer->segments.seg.begin(),myServer->segments.seg.end(),retypesegs[i]);
        if(it==myServer->segments.seg.end()){
            std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
            return;
        }
        int now=QDateTime::currentMSecsSinceEpoch();
        for(auto &unit:it->row){
            unit.type=newcolor;
            unit.level=now-unit.timestamp;
            unit.creatmode=useridx*10+clienttype;
        }
        qDebug()<<"server retypesegment";
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
    qDebug()<<msgs<<"server send msgs:";

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
            auto ps=msg.right(msg.size()-QString("/Login:").size()).split(' ',Qt::SkipEmptyParts);
            if (ps.size()!=2){
                std::cerr<<"login in error:"<<msg.toStdString();
//                this->disconnectFromHost();
//                this->close();//关闭读
                emit exitNow();
                return;
            }
            qDebug()<<"subThread:"<<QThread::currentThreadId();
            receiveuser(ps[0]);
        }else{
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
            }

            mutex.lock();
            myServer->processedmsgcnt+=1;
            mutex.unlock();

            // QString::number按照第二个参数提供的转换进制将数字类型转换为QString
            auto log=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")+QString::number(myServer->processedmsgcnt+myServer->savedmsgcnt)+" "+msg+"\n";
            logfile->write(log.toStdString().c_str(),log.toStdString().size());
            myServer->msglist.append(msg);
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

                    mutex.lock();
                    myServer->receivedcnt+=1;
                    mutex.unlock();

                    std::cout<<QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ").toStdString()<<myServer->receivedcnt<<" receive from "<<username.toStdString()<<" :"<<data<<std::endl;
                    qDebug()<<QString("client read message %1, %2").arg(username).arg(data).toStdString().c_str();
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
    while(this->bytesAvailable())
        onread();
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

void CollClient::receiveuser(const QString user)
{
    username=user;
    if(myServer->hashmap.contains(user))
    {
        std::cerr<<"ERROR:"+user.toStdString()+" is duolicate,will remove the first\n";
//        myServer->hashmap[user]->disconnectFromHost();
          emit myServer->clientDisconnectFromHost(myServer->hashmap[user]);
    }

    myServer->hashmap[user]=this;
    updateuserlist();
    //todo发送保存的文件
    sendfiles({
    myServer->anopath,myServer->apopath,myServer->swcpath
              });
    sendmsgcnt=myServer->savedmsgcnt;
    qDebug()<<"receive user init sendmsgcnt = "<<sendmsgcnt;
    // 获取协同的ano文件名
    QString msg=QString("STARTCOLLABORATE:%1").arg(myServer->anopath.section('/',-1,-1));
    sendmsgs({msg});

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
    if(!this) return;
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
    //这里会超出范围吗
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
        this->disconnectFromHost();
    }
}
