﻿#ifndef COLLCLIENT_H
#define COLLCLIENT_H

#include <QTcpSocket>
#include <basic_c_fun/basic_surf_objs.h>
#include <neuron_editing/neuron_format_converter.h>
#include <iostream>
#include <QFile>

class CollServer;
class CollClient : public QTcpSocket
{
    Q_OBJECT
    struct DataType{
        bool isFile=false;//false msg,true file
        qint64 datasize=0;
    };

    struct segInfoUnit
    {
        segInfoUnit() { hierarchy = 0; }
        long segID;
        long head_tail;
        long nodeCount;
        bool refine;

        int branchID, paBranchID;
        int hierarchy;
    };

public:
    explicit CollClient(qintptr handle,CollServer* curServer,QObject *parent = nullptr);//初始化

    ~CollClient(){
        std::cerr<<username.toStdString()+" is release\n";
    }

    void updateuserlist();//广播在线用户
    void addseg(const QString msg);//加线
    void delseg(const QString msg);//减线
    void delmarkers(const QString msg);//减点
    void retypemarker(const QString msg);//marker改颜色
    void connectseg(const QString msg);//连线
    void retypesegment(const QString msg);//线改颜色

    void simpleConnectExecutor(V_NeuronSWC_list& segments, vector<segInfoUnit>& segInfo);
    static QTimer timerforupdatemsg;//todo
//    static QStringList msglist;
//    static int processedmsgcnt;
//    static int savedmsgcnt;
//    static int receivedcnt;

//    static QMap<QString,CollClient*> hashmap;//user->client
//    static V_NeuronSWC_list segments;
//    static bool isSomaExists;
//    static XYZ somaCoordinate;
//    static QList<CellAPO> markers;

//    static QString swcpath;
//    static QString apopath;
//    static QString anopath;

public slots:
    void sendmsgs(const QStringList &msgs);//发送消息
    void ondisconnect();//连接断开
    void onread();//读数据
    void onError(QAbstractSocket::SocketError); //异常连接
    void addmarkers(const QString msg);//加点

    void updatesendmsgcnt2processed();//将自动保存时已处理还未发送的消息发送
    void sendmsgs2client(const int maxsize=0);//发送信息给客户端
    void quit();
    void disconnectByServer(CollClient* collclient);
private:
    CollServer* myServer;
//    SocketHelper* sockethelper;

    void sendfiles(const QStringList &filenames);//发送文件
    void preprocessmsgs(const QStringList &msgs);//处理消息

    void receiveuser(const QString user);//接收到用户登陆消息
    void resetdatatype();//重置接收数据结构

    DataType datatype;
    QString username="";
    int sendmsgcnt=0;


signals:
    void removeList(QThread*);
    void noUsers();
    void exitNow();

};

#endif // COLLCLIENT_H
