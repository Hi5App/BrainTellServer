#ifndef UTILS_H
#define UTILS_H

#include <QCoreApplication>
#include <QDir>
#include "neuron_editing/neuron_format_converter.h"
#include <hiredis/hiredis.h>
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
};
void dirCheck(QString dirBaseName);
QStringList getSwcInBlock(const QString msg,const V_NeuronSWC_list& testVNL);
QStringList getApoInBlock(const QString msg,const QList <CellAPO>& wholePoint);

void setredis(const int port,const char *ano);
void setexpire(const int port,const char *ano,const int expiretime);

vector<V_NeuronSWC>::iterator findseg(vector<V_NeuronSWC>::iterator begin,vector<V_NeuronSWC>::iterator end,const V_NeuronSWC seg);
NeuronTree convertMsg2NT(QStringList pointlist,int client,int user,int mode=0);

double distance(const CellAPO &m1,const CellAPO &m2);
int findnearest(const CellAPO &m,const QList<CellAPO> &markers);

void init();
#endif // UTILS_H
