#ifndef COLLCLIENT_H
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
    void addmarkers(const QString msg);//加marker
    void delmarkers(const QString msg);//删marker
    void retypemarker(const QString msg);//marker改颜色
    void connectseg(const QString msg);//连线
    void retypesegment(const QString msg);//线改颜色
    void splitseg(const QString msg);//break seg

    void simpleConnectExecutor(V_NeuronSWC_list& segments, vector<segInfoUnit>& segInfo);
    static QTimer timerforupdatemsg;//todo

    //analyze
    void analyzeSomaNearBy(const QString msg);//分析soma处是否连接至一点
    void analyzeColorMutation(const QString msg);//分析是否存在颜色突变
    void analyzeDissociativeSegs(const QString msg);//分析是否存在游离的线段
    void analyzeAngles(const QString msg);//分析树突分叉的角度是否合理

public slots:
    void sendmsgs(const QStringList &msgs);//发送消息
    void ondisconnect();//连接断开
    void onread();//读数据
    void onError(QAbstractSocket::SocketError); //异常连接

    void updatesendmsgcnt2processed();//将自动保存时已处理还未发送的消息发送
    void sendmsgs2client(const int maxsize=0);//发送信息给客户端
    void quit();
    void disconnectByServer(CollClient* collclient);
private:
    CollServer* myServer;
//    SocketHelper* sockethelper;

    void sendfiles(const QStringList &filenames);//发送文件
    void preprocessmsgs(const QStringList &msgs);//处理消息

    void receiveuser(const QString user, QString RES);//接收到用户登陆消息
    void resetdatatype();//重置接收数据结构

    DataType datatype;
    QString username="";
    int sendmsgcnt=0;


signals:
    void removeList(QThread*);
    void noUsers();
    void exitNow();

    void serverStartTimerForDetectLoops();
    void serverStartTimerForDetectOthers();
    void serverStartTimerForDetectTip();
    void serverStartTimerForDetectCrossing();
    void serverStartTimerForHandleTip();

};

#endif // COLLCLIENT_H
