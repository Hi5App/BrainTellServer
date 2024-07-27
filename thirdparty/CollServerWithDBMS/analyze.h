#ifndef ANALYZE_H
#define ANALYZE_H

#include <set>
#include <QtMath>
#include <QVector3D>
#include "utils.h"
#include "colldetection.h"

//获取soma附近的二分叉和多分叉
vector<int> getMulfurcationsCountNearSoma(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments);
//获取颜色突变点
map<string, set<int>> getColorChangedPoints(V_NeuronSWC_list segments);
//获取游离的线的末端点
set<string> getDissociativeSegEndPoints(V_NeuronSWC_list segments);
//获取背景中的漂浮分支标记点
set<string> getDissociativeSegMarkerPoints(QList<NeuronSWC> neuron);
//获取角度错误的树突分叉点
set<string> getAngleErrPoints(float dist_thre, bool isSomaExists, XYZ somaCoordinate, V_NeuronSWC_list& segments, bool needConsiderType);
//计算两个向量的夹角
float calculateAngleofVecs(QVector3D vector1, QVector3D vector2);
//将soma点的半径设置为1.234
bool setSomaPointRadius(QString fileSaveName, V_NeuronSWC_list segments, XYZ somaCoordinate, double dist_thre, CollDetection* detectUtil, QString& msg);
//从swc文件中获取soma点的序号
int getSomaNumberFromSwcFile(QString filePath, float r, QString& msg);

#endif // ANALYZE_H
