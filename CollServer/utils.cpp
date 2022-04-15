#include <QCoreApplication>
#include <QDir>
#include "neuron_editing/neuron_format_converter.h"

#include "utils.h"

void dirCheck(QString dirBaseName)
{
    if(!QDir(QCoreApplication::applicationDirPath()+"/"+dirBaseName).exists())
    {
        QDir(QCoreApplication::applicationDirPath()).mkdir(dirBaseName);
    }
}

QStringList getSwcInBlock(const QString msg,const V_NeuronSWC_list& testVNL)
{
    /*
     * p1:brain_id;res;x;y;z;size;socket.descriptor
     * p2:Neuron_id/name
     * 返回：文件名，文件路径
     */
    QStringList paraList=msg.split(";",Qt::SkipEmptyParts);
    QString brain_id=paraList.at(0).trimmed();//1. tf name/RES  2. .v3draw// test:17302;RES;x;y;z;b
    //0: 18465/RESx18000x13000x5150
    //1: 12520
    //2: 7000
    //3: 2916
    int res=paraList.at(1).toInt();//>0
    int xpos=paraList.at(2).toInt();
    int ypos=paraList.at(3).toInt();
    int zpos=paraList.at(4).toInt();
    int blocksize=paraList.at(5).toInt();

    {
        QString name=brain_id;
        int x1=xpos-blocksize;
        int x2=xpos+blocksize;
        int y1=ypos-blocksize;
        int y2=ypos+blocksize;
        int z1=zpos-blocksize;
        int z2=zpos+blocksize;
        int cnt=pow(2,res-1);

        dirCheck("tmp");
        QString BBSWCNAME=QCoreApplication::applicationDirPath()+"/tmp/blockGet__"+name+QString("__%1__%2__%3__%4__%5__%6__%7.ano.eswc")
                .arg(x1).arg(x2).arg(y1).arg(y2).arg(z1).arg(z2).arg(cnt);
        x1*=cnt;x2*=cnt;y1*=cnt;y2*=cnt;z1*=cnt;z2*=cnt;
        V_NeuronSWC_list tosave;
        for(std::vector<V_NeuronSWC_unit>::size_type i=0;i<testVNL.seg.size();i++)
        {
            NeuronTree SS;
            V_NeuronSWC seg_temp =  testVNL.seg.at(i);
            seg_temp.reverse();
            for(std::vector<V_NeuronSWC_unit>::size_type j=0;j<seg_temp.row.size();j++)
            {
                if(seg_temp.row.at(j).x>=x1&&seg_temp.row.at(j).x<=x2
                        &&seg_temp.row.at(j).y>=y1&&seg_temp.row.at(j).y<=y2
                        &&seg_temp.row.at(j).z>=z1&&seg_temp.row.at(j).z<=z2)
                {
                    tosave.seg.push_back(seg_temp);
                    break;
                }
            }
        }
        qDebug()<<"get nt size:"<<tosave.seg.size();
        auto nt=V_NeuronSWC_list__2__NeuronTree(tosave);
        writeESWC_file(BBSWCNAME,nt);
        return {BBSWCNAME.right(BBSWCNAME.size()-BBSWCNAME.lastIndexOf('/')),BBSWCNAME};
    }
}

QStringList getApoInBlock(const QString msg,const QList <CellAPO>& wholePoint)
{
    /*
     * p1:brain_id;res;x;y;z;size;socket.descriptor
     * p2:Neuron_id/name
     * 返回：文件名，文件路径
     */
    QStringList paraList=msg.split(";",Qt::SkipEmptyParts);
    QString brain_id=paraList.at(0).trimmed();//1. tf name/RES  2. .v3draw// test:17302;RES;x;y;z;b
    //0: 18465/RESx18000x13000x5150
    //1: 12520
    //2: 7000
    //3: 2916
    int res=paraList.at(1).toInt();//>0
    int xpos=paraList.at(2).toInt();
    int ypos=paraList.at(3).toInt();
    int zpos=paraList.at(4).toInt();
    int blocksize=paraList.at(5).toInt();

    {
        QString name=brain_id;
        int x1=xpos-blocksize;
        int x2=xpos+blocksize;
        int y1=ypos-blocksize;
        int y2=ypos+blocksize;
        int z1=zpos-blocksize;
        int z2=zpos+blocksize;
        int cnt=pow(2,res-1);


        dirCheck("tmp");
        QString BBAPONAME=QCoreApplication::applicationDirPath()+"/tmp/blockGet__"+name+QString("__%1__%2__%3__%4__%5__%6__%7.ano.apo")
                .arg(x1).arg(x2).arg(y1).arg(y2).arg(z1).arg(z2).arg(cnt);
        x1*=cnt;x2*=cnt;y1*=cnt;y2*=cnt;z1*=cnt;z2*=cnt;
        qDebug()<<"x1,x2,y1,y2,z1,z2"<<x1<<x2<<y1<<y2<<z1<<z2;
        QList <CellAPO> to_save;
        for(auto marker:wholePoint)
        {
            if(marker.x>=x1&&marker.x<=x2
              &&marker.y>=y1&&marker.y<=y2
              &&marker.z>=z1&&marker.z<=z2)
            {
                to_save.append(marker);
            }
        }
        qDebug()<<"to_save.size()="<<to_save.size();
        writeAPO_file(BBAPONAME,to_save);
        return {BBAPONAME.right(BBAPONAME.size()-BBAPONAME.lastIndexOf('/')),BBAPONAME};
    }
}

void setredis(int port,const char *ano)
{
    // 取消DB0中键ano的过期时间
    redisContext *c = redisConnect("172.18.0.3", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
        } else {
            printf("Can't allocate redis context\n");
        }
    }
    redisReply *reply = (redisReply *)redisCommand(c, "SELECT 0");
    freeReplyObject(reply);

    qDebug()<<QString("PERSIST Ano+Port:%1;%2").arg(ano).arg(port).toStdString().c_str();
    reply=(redisReply *)redisCommand(c, QString("PERSIST Ano+Port:%1;%2").arg(ano).arg(port).toStdString().c_str());
    if(reply->integer!=1){
        exit(-1);
    }
    freeReplyObject(reply);

    redisFree(c);
}

void setexpire(int port,const char *ano,int expiretime)
{
    //设置DB0中键ano的过期时间
    redisContext *c = redisConnect("172.18.0.3", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
        } else {
            printf("Can't allocate redis context\n");
        }
    }
    redisReply *reply = (redisReply *)redisCommand(c, "SELECT 0");
    freeReplyObject(reply);
    qDebug()<<QString("EXPIRE Ano+Port:%1;%2 %3").arg(ano).arg(port).arg(expiretime).toStdString().c_str();
    reply=(redisReply *)redisCommand(c, QString("EXPIRE Ano+Port:%1;%2 %3").arg(ano).arg(port).arg(expiretime*100).toStdString().c_str());
    freeReplyObject(reply);
    redisFree(c);
}
NeuronTree convertMsg2NT(QStringList pointlist,int client,int user,int mode)
{
    NeuronTree newTempNT;
    newTempNT.listNeuron.clear();
    newTempNT.hashNeuron.clear();
    int cnt=pointlist.size();
    int timestamp=QDateTime::currentMSecsSinceEpoch();
    for(int i=0;i<cnt;i++)
    {
        NeuronSWC S;
        QStringList nodelist=pointlist[i].split(' ',Qt::SkipEmptyParts);
        if(nodelist.size()<4) return NeuronTree();
        S.n=i+1;
        S.type=nodelist[0].toUInt();

        S.x=nodelist[1].toFloat();
        S.y=nodelist[2].toFloat();
        S.z=nodelist[3].toFloat();
        switch (mode) {
            case 0:S.r=user*10+client;break;
            case 1:S.r=user;break;
            case 2:S.r=client;break;
        }

        if(i==0) S.pn=-1;
        else S.pn=i;
        S.timestamp=timestamp;
        newTempNT.listNeuron.push_back(S);
        newTempNT.hashNeuron.insert(S.n,newTempNT.listNeuron.size());
    }
    return newTempNT;
}

vector<V_NeuronSWC>::iterator findseg(vector<V_NeuronSWC>::iterator begin,vector<V_NeuronSWC>::iterator end,const V_NeuronSWC seg)
{
    vector<V_NeuronSWC>::iterator result=end;
    double mindist=0.2;
    const std::vector<V_NeuronSWC_unit>::size_type cnt=seg.row.size();

    while(begin!=end)
    {
        if(begin->row.size()==cnt)
        {
            double dist=0;
            for(std::vector<V_NeuronSWC_unit>::size_type i=0;i<cnt;i++)
            {
                auto node=begin->row.at(i);
                dist+=sqrt(
                           pow(node.x-seg.row[i].x,2)
                          +pow(node.y-seg.row[i].y,2)
                          +pow(node.z-seg.row[i].z,2)
                           );
            }
            if(dist/cnt<mindist)
            {
                mindist=dist;
                result=begin;
            }

            dist=0;
            for(std::vector<V_NeuronSWC_unit>::size_type i=0;i<cnt;i++)
            {
                auto node=begin->row.at(i);
                dist+=sqrt(
                           pow(node.x-seg.row[cnt-i-1].x,2)
                          +pow(node.y-seg.row[cnt-i-1].y,2)
                          +pow(node.z-seg.row[cnt-i-1].z,2)
                           );
            }
            if(dist/cnt<mindist)
            {
                mindist=dist;
                result=begin;
            }
        }
        begin++;
    }
    return result;
}

double distance(const CellAPO &m1,const CellAPO &m2)
{
    return sqrt(
                (m1.x-m2.x)*(m1.x-m2.x)+
                (m1.y-m2.y)*(m1.y-m2.y)+
                (m1.z-m2.z)*(m1.z-m2.z)
            );
}

int findnearest(const CellAPO &m,const QList<CellAPO> &markers)
{
    int index=-1;
    double thres=1;
    for(int i=0;i<markers.size();i++){
        if(distance(m,markers[i])<thres){
            index=i;
        }
    }
    return index;
}

void init()
{
    for(int i=0;i<10;i++){
        for(int usercnt=100;usercnt<=100;usercnt+=10){
            QDir dir("/");
            for(int msgcnt=100;msgcnt<=100;msgcnt+=10){
                auto anoname=QString("/Users/huanglei/Desktop/dataserver/data/18454/18454_00049/pressure_18454_00049_%1_%2_%3/pressure_18454_00049_%1_%2_%3.ano")
                        .arg(usercnt).arg(msgcnt).arg(4000+i);
                QFile anof(anoname),apof(anoname+".apo"),swcf(anoname+".eswc");
                QString data=QString("APOFILE=pressure_18454_00049_%1_%2_%3.ano.apo\nSWCFILE=pressure_18454_00049_%1_%2_%3.ano.eswc").arg(usercnt).arg(msgcnt).arg(4000+i);
                dir.mkdir(QString("/Users/huanglei/Desktop/dataserver/data/18454/18454_00049/pressure_18454_00049_%1_%2_%3").arg(usercnt).arg(msgcnt).arg(4000+i));
                if(anof.open(QIODevice::WriteOnly)){
                    anof.write(data.toStdString().c_str(),data.size());
                    anof.close();
                }
                if(apof.open(QIODevice::WriteOnly)){
                    apof.close();
                }
                if(swcf.open(QIODevice::WriteOnly)){
                    swcf.close();
                }
            }
        }
    }

}
