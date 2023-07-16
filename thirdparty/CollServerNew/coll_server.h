#ifndef COLL_SERVER_H
#define COLL_SERVER_H

#include <QTimer>
#include <QTcpServer>
#include <vector>
#include <set>
#include "collclient.h"
#include "collthread.h"
#include <QNetworkAccessManager>

class CollServer:public QTcpServer
{
    Q_OBJECT
public:
    CollServer(QString port,QString image,QString neuron,QString ananame,QString prefix,QObject *parent=nullptr);
    virtual ~CollServer();
    void incomingConnection(qintptr handle);
    XYZ getSomaCoordinate(QString apoPath);
    vector<NeuronSWC> specStructsDetection(V_NeuronSWC_list inputSegList, double dist_thresh=3);
    vector<NeuronSWC> tipDetection(V_NeuronSWC_list inputSegList, double dist_thresh=20);
    vector<NeuronSWC> crossingDetection(V_NeuronSWC_list inputSegList);
    void handleMulFurcation(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleLoop(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleNearBifurcation(vector<NeuronSWC>& bifurPoints, int& count);
    void handleTip(vector<NeuronSWC>& tipPoints);
    void handleCrossing(vector<NeuronSWC>& crossingPoints);

    void addmarkers(const QString msg);
    static CollServer* getInstance();

    static QStringList msglist;
    static int processedmsgcnt;
    static int savedmsgcnt;
    static int receivedcnt;

    static QMap<QString,CollClient*> hashmap;//user->client
    static V_NeuronSWC_list segments;
    static bool isSomaExists;
    static XYZ somaCoordinate;
    static QList<CellAPO> markers;

    static QString swcpath;
    static QString apopath;
    static QString anopath;

    static QMutex mutex;
    static QString RES;

signals:
    void clientAddMarker(QString);
    void clientSendMsgs(QStringList);
    void clientUpdatesendmsgcnt();
    void clientSendmsgs2client(int);
    void clientDeleteLater();
    void clientDisconnectFromHost(CollClient*);

public slots:
    void detectOthers();
    void detectTips();
    void detectCrossings();
    void imediateSave();
    void autoSave();
    void autoExit();

    void RemoveList(QThread* thread);
private:
//    qsizetype idxforprocessed=0;
    QString Port;
    QString Image;
    QString Neuron;
    QString AnoName;
    QString Prefix;

    QTimer *timerForAutoSave;
    QTimer *timerForDetectOthers;
    QTimer *timerForDetectTip;
    QTimer *timerForDetectCrossing;
    QTimer *timerForAutoExit;
    static CollServer* curServer;
    QList<CollThread*> list_thread;
    QNetworkAccessManager* accessManager;
    QString HostAddress;
};

#endif // COLL_SERVER_H
