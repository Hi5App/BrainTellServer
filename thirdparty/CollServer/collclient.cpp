#include "collclient.h"
#include "utils.h"
//#include <algorithm>
extern QFile* logfile;


QTimer CollClient::timerforupdatemsg;
QStringList CollClient::msglist;
int CollClient::processedmsgcnt=0;
int CollClient::savedmsgcnt=0;
int CollClient::receivedcnt=0;
QMap<QString,CollClient*> CollClient::hashmap;
V_NeuronSWC_list CollClient::segments;
QList<CellAPO> CollClient::markers;
 int receivedcnt;
QString CollClient::swcpath;
QString CollClient::apopath;
QString CollClient::anopath;

CollClient:: CollClient(qintptr handle,QObject *parent ):QTcpSocket(parent){
    setSocketDescriptor(handle);
    connect(this,&QTcpSocket::readyRead,this,&CollClient::onread);
    connect(this,&QTcpSocket::disconnected,this,&CollClient::ondisconnect);
//    如果一分钟内好没有登陆，则断开连接
    QTimer::singleShot(60*1000,this,[this]{
        if(username==""){
            this->disconnectFromHost();
        }
    });
    setSocketOption(QAbstractSocket::KeepAliveOption,1);//keepalive


}

void CollClient::updateuserlist()
{
    auto users=CollClient::hashmap.keys();
    QString msg="/activeusers:"+users.join(',');
    for (auto iter=CollClient::hashmap.begin();iter!=CollClient::hashmap.end();iter++){
        qDebug()<<"user:"<<iter.key()<<" state:"<<iter.value()->state();
    }
    for(auto &user:users){
        hashmap[user]->sendmsgs({msg});
    }
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

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }
    auto addnt=convertMsg2NT(pointlist,clienttype,useridx);
    segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
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
    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }
    auto delnt=convertMsg2NT(pointlist,clienttype,useridx);
    auto delseg=NeuronTree__2__V_NeuronSWC_list(delnt).seg[0];

    auto it=findseg(segments.seg.begin(),segments.seg.end(),delseg);
    if(it!=segments.seg.end())
        segments.seg.erase(it);
    else
        std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
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
    auto addnt=convertMsg2NT(pointlist,clienttype,useridx);
    segments.append(NeuronTree__2__V_NeuronSWC_list(addnt).seg[0]);
}

void CollClient::addmarkers(const QString msg)
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
    for(auto &msg:pointlist){
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=4) continue;
        marker.color.r=neuron_type_color[markerinfo[0].toUInt()][0];
        marker.color.g=neuron_type_color[markerinfo[0].toUInt()][1];
        marker.color.b=neuron_type_color[markerinfo[0].toUInt()][2];
        marker.x=markerinfo[1].toDouble();
        marker.y=markerinfo[2].toDouble();
        marker.z=markerinfo[3].toDouble();
        markers.append(marker);
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
        idx=-1;
        auto markerinfo=msg.split(' ',Qt::SkipEmptyParts);
        if(markerinfo.size()!=4) continue;
        marker.color.r=neuron_type_color[markerinfo[0].toUInt()][0];
        marker.color.g=neuron_type_color[markerinfo[0].toUInt()][1];
        marker.color.b=neuron_type_color[markerinfo[0].toUInt()][2];
        marker.x=markerinfo[1].toDouble();
        marker.y=markerinfo[2].toDouble();
        marker.z=markerinfo[3].toDouble();
        idx=findnearest(marker,markers);
        if(idx!=-1) markers.removeAt(idx);
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

    QStringList pointlist=pointlistwithheader;
    pointlist.removeAt(0);
    if(pointlist.size()==0){
        std::cerr<<"ERROR:pointlist.size=0\n";
    }

    auto delnt=convertMsg2NT(pointlist,clienttype,useridx);
    auto delseg=NeuronTree__2__V_NeuronSWC_list(delnt).seg[0];

    auto it=findseg(segments.seg.begin(),segments.seg.end(),delseg);
    if(it==segments.seg.end()){
        std::cerr<<"INFO:not find del seg ,"<<msg.toStdString()<<std::endl;
        return;
    }
    int now=QDateTime::currentMSecsSinceEpoch();
    for(auto &unit:it->row){
        unit.type=newcolor;
        unit.level=now-unit.timestamp;
        unit.creatmode=useridx*10+clienttype;
    }
}

void CollClient::sendmsgs(const QStringList &msgs)
{
//    if(!this) return;
    if(this->state()!=QAbstractSocket::ConnectedState){
        qDebug()<<"error: send msg to "<<this->username<<",but connect is "<<this->state();
        ondisconnect();
        return;
    }

    const std::string data=msgs.join(';').toStdString();
    const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
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
                this->disconnectFromHost();
                return;
            }
            receiveuser(ps[0]);
        }else{
            if(msg.startsWith("/drawline_norm:")||msg.startsWith("/drawline_redo:")){
                addseg(msg.right(msg.size()-QString("/drawline_norm:").size()));
            }else if(msg.startsWith("/delline_norm:")||msg.startsWith("/delline_redo:")){
                delseg(msg.right(msg.size()-QString("/delline_norm:").size()));
            }else if(msg.startsWith("/addmarker_norm:")||msg.startsWith("/addmarker_redo:")){
                addmarkers(msg.right(msg.size()-QString("/addmarker_norm:").size()));
            }else if(msg.startsWith("/delmarker_norm:")||msg.startsWith("/delmarker_redo:")){
                delmarkers(msg.right(msg.size()-QString("/delmarker_norm:").size()));
            }else if(msg.startsWith("/connectline_norm:")||msg.startsWith("/connectline_redo:")){
                connectseg(msg.right(msg.size()-QString("/connectline_norm:").size()));
            }else if(msg.startsWith("/retypeline_norm:")||msg.startsWith("/retypeline_redo:")){
                retypesegment(msg.right(msg.size()-QString("/retypeline_norm:").size()));
            }
            CollClient::processedmsgcnt+=1;
            auto log=QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")+QString::number(CollClient::processedmsgcnt+CollClient::savedmsgcnt)+" "+msg+"\n";
            logfile->write(log.toStdString().c_str(),log.toStdString().size());
            msglist.append(msg);
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
                    if(!msg.startsWith("DataTypeWithSize:")){
                        this->write({"Socket Receive ERROR!"});
                        std::cerr<<username.toStdString()+" receive not match format\n";
                    }

                    auto ps=msg.right(msg.size()-QString("DataTypeWithSize:").size()).split(' ');
                    if (ps[0].toInt()!=0){
                        this->write({"Socket Receive ERROR!"});
                        std::cerr<<username.toStdString()+" receive not match format\n";
                    }
                    datatype.isFile=ps[0].toUInt();
                    datatype.datasize=ps[1].toUInt();
                }else{
                    break;
                }
            }else{
                //已经接收了头，正在准备接收 消息数据
                if(bytesAvailable()>=datatype.datasize){
                    char *data=new char[datatype.datasize+1];
                    this->read(data,datatype.datasize);
                    data[datatype.datasize]='\0';
                    std::cout<<QDateTime::currentDateTime().toString(" yyyy/MM/dd hh:mm:ss ").toStdString()<<(++receivedcnt)<<" receive from "<<username.toStdString()<<" :"<<data<<std::endl;
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
    this->flush();
    while(this->bytesAvailable())
        onread();
    this->close();//关闭读
    if(CollClient::hashmap.contains(username)&&CollClient::hashmap[username]==this)
        CollClient::hashmap.remove(username);
    CollClient::updateuserlist();
    this->deleteLater();
}

void CollClient::receiveuser(const QString user)
{
    username=user;
    if(CollClient::hashmap.contains(user))
    {
        std::cerr<<"ERROR:"+user.toStdString()+" is duolicate,will remove the first\n";
        CollClient::hashmap[user]->disconnectFromHost();
    }
    CollClient::hashmap[user]=this;
    CollClient::updateuserlist();
    //todo发送保存的文件
    sendfiles({
    anopath,apopath,swcpath
              });
    sendmsgcnt=CollClient::savedmsgcnt;
    QString msg=QString("STARTCOLLABORATE:%1").arg(anopath.section('/',-1,-1));
    sendmsgs({msg});
}



void CollClient::updatesendmsgcnt2processed()
{
    if(sendmsgcnt<CollClient::processedmsgcnt)
    {
        sendmsgs(QStringList(CollClient::msglist.begin()+this->sendmsgcnt,
                             CollClient::msglist.begin()+CollClient::processedmsgcnt));
        sendmsgcnt=CollClient::processedmsgcnt;
    }
//    sendmsgcnt-=CollClient::processedmsgcnt;
}

void CollClient::sendmsgs2client(int maxsize)
{
    if(!this) return;
    if(CollClient::msglist.size()<=sendmsgcnt){
        qDebug()<<"msglist.size="<<msglist.size()<<" sendmsgcnt="<<sendmsgcnt;
        return;
    }
    auto end=MIN(int(CollClient::msglist.size()),int(this->sendmsgcnt+maxsize));
//    if(maxsize>0)
//        maxsize=MIN(maxsize,CollClient::msglist.size()-sendmsgcnt);
//    else
//        maxsize=CollClient::msglist.size()-sendmsgcnt;
    qDebug()<<"send to "<< this->username<<" :("<<CollClient::msglist.begin()+this->sendmsgcnt
           <<","<<CollClient::msglist.begin()+end<<")/"<<CollClient::msglist.size();
    sendmsgs(QStringList(CollClient::msglist.begin()+this->sendmsgcnt,
                         CollClient::msglist.begin()+end));
    this->sendmsgcnt+=maxsize;
}
void CollClient::resetdatatype()
{
    datatype.isFile=false;
    datatype.datasize=0;
}

