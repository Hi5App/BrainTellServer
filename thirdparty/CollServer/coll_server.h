#ifndef COLL_SERVER_H
#define COLL_SERVER_H

#include <QTimer>
#include <QTcpServer>
#include <vector>
#include <set>
#include "collclient.h"
#include "collthread.h"
#include <QNetworkAccessManager>
#include "colldetection.h"

class CollServer:public QTcpServer
{
    Q_OBJECT
public:
    CollServer(QString port,QString image,QString neuron,QString ananame,QString prefix,QObject *parent=nullptr);
    virtual ~CollServer();
    void incomingConnection(qintptr handle);

    void addmarkers(const QString msg);
    CollDetection* detectUtil;
    static CollServer* getInstance();

    QStringList msglist;
    int processedmsgcnt;
    int savedmsgcnt;
    int receivedcnt;

    QMap<QString,CollClient*> hashmap;//user->client
    V_NeuronSWC_list segments;

    QList<CellAPO> markers;

    QString swcpath;
    QString apopath;
    QString anopath;

    QMutex mutex;
    QString RES;

    bool isSomaExists;
    XYZ somaCoordinate;

signals:
//    void clientAddMarker(QString);
    void clientSendMsgs(QStringList);
    void clientUpdatesendmsgcnt();
    void clientSendmsgs2client(int);
    void clientDeleteLater();
    void clientDisconnectFromHost(CollClient*);

public slots:
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
    QTimer *timerForDetectLoops;
    QTimer *timerForDetectOthers;
    QTimer *timerForDetectTip;
    QTimer *timerForDetectCrossing;
    QTimer *timerForAutoExit;
    static CollServer* curServer;
    QList<CollThread*> list_thread;

public:
    QString getAnoName();
    QString getImage();
};

#endif // COLL_SERVER_H
