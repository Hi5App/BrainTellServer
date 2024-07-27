/*
 *  sort_func.cpp
 *  core functions for sort neuron swc
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Changed by  Wan, Yinan, on 06/23/11.
 *  Enable processing of .ano file, add threshold parameter by Yinan Wan, on 01/31/12
 */
#ifndef __SORT_SWC_H_
#define __SORT_SWC_H_

//#include <windows.h>

#include <QtGlobal>
#include <math.h>
//#include <unistd.h> //remove the unnecessary include file. //by PHC 20131228
#include "basic_c_fun/basic_surf_objs.h"
#include "neuron_editing/neuron_format_converter.h"
#include <string.h>
#include <vector>
#include <iostream>
#include "fstream"
#include <set>

using namespace std;

//class MessageBox : public QMessageBox
//{
//public:
//    int timeout;
//    bool autoClose;
//    int currentTime;

//    void MessageBox::showEvent ( QShowEvent * event ) {
//        currentTime = 0;
//        if (autoClose) {
//        this->startTimer(1000);
//        }
//    }

//    void MessageBox::timerEvent(QTimerEvent *event)
//    {
//        currentTime++;
//        if (currentTime>=timeout) {
//        this->done(0);
//        }
//    }
//};

#ifndef VOID
#define VOID 1000000000
#endif
#ifndef MAX_INT
#define MAX_INT 10000000
#endif
#ifndef SEPARATOR
#define SEPARATOR "_"
#endif

//#define PI 3.14159265359
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

bool fexists(QString filename);

QVector< QVector<V3DLONG> > get_neighbors(QList<NeuronSWC> &neurons, const QHash<V3DLONG,V3DLONG> & LUT);

QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons, QHash<V3DLONG, NeuronSWC> & LUT_newid_to_node);

QHash<V3DLONG, V3DLONG> getUniqueLUT_updated(QList<NeuronSWC> &neurons, QHash<V3DLONG, NeuronSWC> & LUT_newid_to_node);

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);

bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);

bool SortSWCSimplify(QList<NeuronSWC> & neurons, V_NeuronSWC_list segments, QList<NeuronSWC> & result, V3DLONG newrootid, QString& msg, vector<NeuronSWC>& loopMarkers);

set<string> getTreeMarkerPoints(QList<NeuronSWC> & neurons);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);

void connect_swc(NeuronTree nt,QList<NeuronSWC>& newNeuron, double disThr,double angThr);

NeuronTree pruneswc(NeuronTree nt, double length);

#endif
