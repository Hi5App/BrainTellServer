#ifndef ANALYZE_H
#define ANALYZE_H

#include <set>
#include <QtMath>
#include <QVector3D>
#include "utils.h"

//获取soma附近的二分叉和多分叉
vector<int> getMulfurcationsCountNearSoma(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments);
//获取颜色突变点
map<string, set<int>> getColorChangedPoints(V_NeuronSWC_list segments);
//获取游离的线的末端点
set<string> getDissociativeSegEndPoints(V_NeuronSWC_list segments);
//获取角度错误的树突分叉点
set<string> getAngleErrPoints(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments);

//计算两个向量的夹角
float calculateAngleofVecs(QVector3D vector1, QVector3D vector2);

#endif // ANALYZE_H
