#ifndef COLLDETECTION_H
#define COLLDETECTION_H
#include <vector>
#include <set>
#include <utils.h>
#include <basic_c_fun/basic_surf_objs.h>
#include <neuron_editing/neuron_format_converter.h>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <algorithm>

class CollServer;
class CollDetection : public QObject
{
    Q_OBJECT
private:
    CollServer* myServer;
    QNetworkAccessManager* accessManager;
    QString HostAddress;

public:
    static bool isSomaExists;
    static XYZ somaCoordinate;

    explicit CollDetection(CollServer* curServer, QObject* parent=nullptr);
    ~CollDetection(){}
    XYZ getSomaCoordinate(QString apoPath);
    vector<NeuronSWC> specStructsDetection(V_NeuronSWC_list inputSegList, double dist_thresh=3);
    vector<NeuronSWC> tipDetection(V_NeuronSWC_list inputSegList, double dist_thresh=20);
    vector<NeuronSWC> crossingDetection(V_NeuronSWC_list inputSegList);
    void handleMulFurcation(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleLoop(vector<NeuronSWC>& outputSpecialPoints, int& count);
    void handleNearBifurcation(vector<NeuronSWC>& bifurPoints, int& count);
    void handleTip(vector<NeuronSWC>& tipPoints);
    void handleCrossing(vector<NeuronSWC>& crossingPoints);

public slots:
    void detectOthers();
    void detectTips();
    void detectCrossings();

};

#endif // COLLDETECTION_H
