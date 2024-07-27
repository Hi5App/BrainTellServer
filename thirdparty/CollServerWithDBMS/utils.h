#ifndef UTILS_H
#define UTILS_H

#include <QCoreApplication>
#include <QDir>
#include <set>
#include "neuron_editing/neuron_format_converter.h"
#include "include/hiredis/hiredis.h"
const int neuron_type_color[21][3] = {
    {255, 255, 255},  // white,   0-undefined
    {20,  20,  20 },  // black,   1-soma
    {200, 20,  0  },  // red,     2-axon
    {0,   20,  200},  // blue,    3-dendrite
    {200, 0,   200},  // purple,  4-apical dendrite
    //the following is Hanchuan's extended color. 090331
    {0,   200, 200},  // cyan,    5
    {220, 200, 0  },  // yellow,  6
    {0,   200, 20 },  // green,   7
    {188, 94,  37 },  // coffee,  8
    {180, 200, 120},  // asparagus,	9
    {250, 100, 120},  // salmon,	10
    {120, 200, 200},  // ice,		11
    {100, 120, 200},  // orchid,	12
    //the following is Hanchuan's further extended color. 111003
    {255, 128, 168},  //	13
    {128, 255, 168},  //	14
    {128, 168, 255},  //	15
    {168, 255, 128},  //	16
    {255, 168, 128},  //	17
    {168, 128, 255}, //	18
    {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
    //the following (20-275) is used for matlab heat map. 120209 by WYN
    {0,0,131}, //20
};

const vector<QString> quality_control_types = {
    "Multifurcation",
    "Approaching bifurcation",
    "Loop",
    "Missing",
    "Crossing error",
    "Color mutation",
    "Dissociative seg",
    "Angle error"
};

void dirCheck(QString dirBaseName);
QStringList getSwcInBlock(const QString msg,const V_NeuronSWC_list& testVNL);
QStringList getApoInBlock(const QString msg,const QList <CellAPO>& wholePoint);

void setredis(const int port,const char *ano);
void setexpire(const int port,const char *ano,const int expiretime);
void recoverPort(const int port);

vector<V_NeuronSWC>::iterator findseg(vector<V_NeuronSWC>::iterator begin,vector<V_NeuronSWC>::iterator end,const V_NeuronSWC seg);
NeuronTree convertMsg2NT(QStringList pointlist,int client,int user, int isMany, int mode=0);

double distance(const CellAPO &m1,const CellAPO &m2);
double distance(const double x1, const double x2, const double y1, const double y2, const double z1, const double z2);
double getSegLength(V_NeuronSWC &seg);
double getPartOfSegLength(V_NeuronSWC &seg, int index);
double getSegLengthBetweenIndexs(V_NeuronSWC &seg, int low, int high);
int findnearest(const CellAPO &m,const QList<CellAPO> &markers);

void init();
void stringToXYZ(string xyz, float& x, float& y, float& z);
void getSegmentsForOthersDetect(V_NeuronSWC_list& last1MinSegments, V_NeuronSWC_list& segmentsForOthersDetect, V_NeuronSWC_list segments);
void getSegmentsForMissingDetect(V_NeuronSWC_list& last3MinSegments, V_NeuronSWC_list& segmentsForMissingDetect, V_NeuronSWC_list segments);
void reverseSeg(V_NeuronSWC& seg);
int getPointInSegIndex(string point, V_NeuronSWC& seg);
map<string, set<size_t>> getWholeGrid2SegIDMap(V_NeuronSWC_list inputSegments);
int isOverlapOfTwoSegs(V_NeuronSWC& seg1, V_NeuronSWC& seg2);
QStringList V_NeuronSWCToSendMSG(V_NeuronSWC seg);

RGB8 getColorFromType(int type);
std::vector<std::string> stringSplit(const std::string&str, char delim);
set<int> getQCMarkerNearBy(vector<V_NeuronSWC> &segs, const QList<CellAPO> &markers);

#endif // UTILS_H
