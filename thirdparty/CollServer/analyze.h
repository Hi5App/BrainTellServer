#ifndef ANALYZE_H
#define ANALYZE_H

#include <set>
#include "utils.h"

//获取soma附近的二分叉和多分叉
vector<int> getMulfurcationsCountNearSoma(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments);
//获取颜色突变点
map<string, set<int>> getColorChangedPoints(V_NeuronSWC_list segments);
//获取游离的线的末端点
set<string> getDissociativeSegEndPoints(V_NeuronSWC_list segments);

#endif // ANALYZE_H
