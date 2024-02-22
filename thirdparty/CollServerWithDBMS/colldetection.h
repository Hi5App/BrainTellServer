﻿#ifndef COLLDETECTION_H
#define COLLDETECTION_H
#include <vector>
#include <set>
#include <unordered_set>
#include "utils.h"
#include "basic_c_fun/basic_surf_objs.h"
#include "neuron_editing/neuron_format_converter.h"
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include "json.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <algorithm>
#include <cmath>

class CollServer;
class CollDetection : public QObject
{
    Q_OBJECT
private:
    CollServer* myServer;
    QNetworkAccessManager* accessManager;
    QString SuperUserHostAddress;
    QString BrainTellHostAddress;
    unordered_set<string> detectedTipPoints;
    set<set<string>> detectedCrossingPoints;
    vector<NeuronSWC> tipPoints;

public:
    static XYZ maxRes;
    static XYZ subMaxRes;
    QTimer *timerForFilterTip;

    explicit CollDetection(CollServer* curServer, string serverIp, string brainServerPort, QObject* parent=nullptr);
    ~CollDetection(){}
    XYZ getSomaCoordinate(QString apoPath);
    vector<NeuronSWC> specStructsDetection(V_NeuronSWC_list& inputSegList, double dist_thresh=1.5);
    vector<NeuronSWC> loopDetection(V_NeuronSWC_list& inputSegList);
    vector<NeuronSWC> tipDetection(V_NeuronSWC_list inputSegList, bool flag, map<string, set<size_t>> allPoint2SegIdMap, double dist_thresh=30);
    QJsonArray crossingDetection();
    vector<NeuronSWC> branchingDetection(V_NeuronSWC_list& inputSegList);
    void handleMulFurcation(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleLoop(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleNearBifurcation(vector<NeuronSWC>& bifurPoints, int& count);
    void handleTip(vector<NeuronSWC>& tipPoints);
    void filterTip(vector<NeuronSWC>& markpoints);
    void handleCrossing(QJsonArray& json);

    void sortSWC(QString fileOpenName, QString fileSaveName, double thres=1000000000, V3DLONG rootid=1000000000);
    void setSWCRadius(QString filePath, int r);
    void getImageRES();
    void getImageMaxRES();
    void getApoForCrop(QString fileSaveName, vector<NeuronSWC> tipPoints);
    void removeShortSegs(V_NeuronSWC_list inputSegList);
    void removeOverlapSegs(V_NeuronSWC_list inputSegList);

signals:
    void removeErrorSegsDone();

public slots:
    void detectWholeAtStart();
    void detectOthers();
    void detectLoops();
    void detectTips();
    void detectTipsWhole();
    void detectCrossings();
    void detectOthersWhole();
    void removeErrorSegs(bool);

};

#endif // COLLDETECTION_H
