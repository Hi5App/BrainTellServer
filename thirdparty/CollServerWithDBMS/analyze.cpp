#include "analyze.h"
#include "sort_swc.h"

vector<int> getMulfurcationsCountNearSoma(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments){
    vector<int> counts;

    map<string, set<size_t> > wholeGrid2SegIDMap;
    map<string, bool> isEndPointMap;

    set<string> allPoints;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2SegIDMap[gridKey].insert(size_t(i));
            allPoints.insert(gridKey);

            if(j == 0 || j == seg.row.size() - 1){
                isEndPointMap[gridKey] = true;
            }
        }
    }

    //末端点和分叉点
    vector<string> points;
    vector<set<int>> linksIndex;

    map<string,int> pointsIndexMap;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(j==0 || j==seg.row.size()-1){
                //在pointsIndexMap中找不到某个线的末端点
                if(pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }else{
                if(wholeGrid2SegIDMap[gridKey].size()>1 &&
                    isEndPointMap.find(gridKey) != isEndPointMap.end() &&
                    pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }
        }
    }
    qDebug()<<"points size: "<<points.size();

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        vector<int> segIndexs;
        set<int> segIndexsSet;
        segIndexs.clear();
        segIndexsSet.clear();
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(pointsIndexMap.find(gridKey) != pointsIndexMap.end()){
                int index = pointsIndexMap[gridKey];
                if(segIndexsSet.find(index) == segIndexsSet.end()){
                    segIndexs.push_back(index);
                    segIndexsSet.insert(index);
                }
            }
        }
        //        qDebug()<<"i : "<<i<<"seg size: "<<seg.row.size()<<" segIndexsSize: "<<segIndexs.size();
        for(size_t j=0; j<segIndexs.size()-1; ++j){
            if(segIndexs[j] == 1 || segIndexs[j+1] == 1){
                qDebug()<<segIndexs[j]<<" "<<segIndexs[j+1];
            }
            linksIndex[segIndexs[j]].insert(segIndexs[j+1]);
            //            linksIndexVec[segIndexs[j]].push_back(segIndexs[j+1]);
            linksIndex[segIndexs[j+1]].insert(segIndexs[j]);
            //            linksIndexVec[segIndexs[j+1]].push_back(segIndexs[j]);
        }
    }

    qDebug()<<"link map end";

    int biCount=0;
    int mulCount=0;
    int othersCount=0;

    for(size_t i=0; i<points.size(); ++i){
        if(linksIndex[i].size() > 3){
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)<dist_thre)
                mulCount++;
        }else if(linksIndex[i].size() == 3){
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)<dist_thre)
                biCount++;
        }else if(linksIndex[i].size()==2){
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)<dist_thre)
                othersCount++;
        }
    }

    counts.push_back(othersCount);
    counts.push_back(biCount);
    counts.push_back(mulCount);
    qDebug()<<"counts: "<<counts[0]<<" "<<counts[1]<<" "<<counts[2];
    return counts;

}

map<string, set<int>> getColorChangedPoints(V_NeuronSWC_list segments){
    map<string, set<int>> point2TypeMap;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            point2TypeMap[gridKey].insert(seg.row[j].type);
        }
    }

    map<string, set<int>> specPointsMap;
    for(auto it=point2TypeMap.begin(); it!=point2TypeMap.end(); it++){
        if(it->second.size()<=1)
            continue;
        specPointsMap[it->first]=it->second;
    }

    return specPointsMap;
}

set<string> getDissociativeSegEndPoints(V_NeuronSWC_list segments){
    map<string, set<size_t> > wholeGrid2SegIDMap;

    set<string> dissociativePoints;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2SegIDMap[gridKey].insert(size_t(i));

        }
    }

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        bool flag=true;
        string savedgridKey;

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            int size=wholeGrid2SegIDMap[gridKey].size();

            if(j == 0)
                savedgridKey=gridKey;
            if(size > 1){
                flag = false;
            }
        }

        if(flag){
            dissociativePoints.insert(savedgridKey);
        }
    }

    return dissociativePoints;
}

set<string> getDissociativeSegMarkerPoints(QList<NeuronSWC> neuron){
    return getTreeMarkerPoints(neuron);
}

set<string> getAngleErrPoints(float dist_thre, bool isSomaExists, XYZ somaCoordinate, V_NeuronSWC_list& segments, bool needConsiderType){
    set<string> angleErrPoints;

    if(!isSomaExists){
        return angleErrPoints;
    }
    map<string, set<int>> point2TypeMap;
    map<string, set<string>> parentMap;
    map<string, set<string>> childMap;
    map<string, set<size_t> > wholeGrid2SegIDMap;
    map<string, bool> isEndPointMap;
    set<string> allPoints;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];

        vector<int> rowN2Index(seg.row.size()+1);

        for(size_t j=0; j<seg.row.size(); ++j){
            rowN2Index[seg.row[j].n]=j;
        }

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            point2TypeMap[gridKey].insert(seg.row[j].type);
            wholeGrid2SegIDMap[gridKey].insert(size_t(i));
            allPoints.insert(gridKey);

            if(j == 0 || j == seg.row.size() - 1){
                isEndPointMap[gridKey] = true;
            }

            if(seg.row[j].parent!=-1){
                float x2Label=seg.row[rowN2Index[seg.row[j].parent]].x;
                float y2Label=seg.row[rowN2Index[seg.row[j].parent]].y;
                float z2Label=seg.row[rowN2Index[seg.row[j].parent]].z;
                QString parentKeyQ=QString::number(x2Label) + "_" + QString::number(y2Label) + "_" + QString::number(z2Label);
                string parentKey=parentKeyQ.toStdString();
                parentMap[gridKey].insert(parentKey);
                childMap[parentKey].insert(gridKey);
            }
        }
    }

    //末端点和分叉点
    vector<string> points;
    vector<set<int>> linksIndex;

    map<string,int> pointsIndexMap;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(j==0 || j==seg.row.size()-1){
                //在pointsIndexMap中找不到某个线的末端点
                if(pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }else{
                if(wholeGrid2SegIDMap[gridKey].size()>1 &&
                    isEndPointMap.find(gridKey) != isEndPointMap.end() &&
                    pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }
        }
    }
    qDebug()<<"points size: "<<points.size();

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        vector<int> segIndexs;
        set<int> segIndexsSet;
        segIndexs.clear();
        segIndexsSet.clear();
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(pointsIndexMap.find(gridKey) != pointsIndexMap.end()){
                int index = pointsIndexMap[gridKey];
                if(segIndexsSet.find(index) == segIndexsSet.end()){
                    segIndexs.push_back(index);
                    segIndexsSet.insert(index);
                }
            }
        }
        //        qDebug()<<"i : "<<i<<"seg size: "<<seg.row.size()<<" segIndexsSize: "<<segIndexs.size();
        for(size_t j=0; j<segIndexs.size()-1; ++j){
            if(segIndexs[j] == 1 || segIndexs[j+1] == 1){
                qDebug()<<segIndexs[j]<<" "<<segIndexs[j+1];
            }
            linksIndex[segIndexs[j]].insert(segIndexs[j+1]);
            //            linksIndexVec[segIndexs[j]].push_back(segIndexs[j+1]);
            linksIndex[segIndexs[j+1]].insert(segIndexs[j]);
            //            linksIndexVec[segIndexs[j+1]].push_back(segIndexs[j]);
        }
    }

    qDebug()<<"link map end";

    set<string> bifurcationPoints;
    for(size_t i=0; i<points.size(); ++i){
        if(linksIndex[i].size() == 3){
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            if(!isSomaExists)
                bifurcationPoints.insert(points[i]);
            else if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)>dist_thre)
                bifurcationPoints.insert(points[i]);
        }
    }

    map<string, vector<int>> twoSegsMap;
    map<string, vector<int>> threeSegsMap;
    for(auto it=bifurcationPoints.begin(); it!=bifurcationPoints.end();){
        set<int> types = point2TypeMap[*it];
        //        qDebug()<<"types: "<<point2TypeMap[*it].size();
        bool flag = true;
        if(needConsiderType){
            if (types.size()!=1) {
                it = bifurcationPoints.erase(it); // 通过迭代器去除元素，并返回下一个有效迭代器
                flag = false;
            }
            else if(*types.begin()!=3){
                it = bifurcationPoints.erase(it);
                flag = false;
            }
        }
        if(flag){
            set<size_t> segIds = wholeGrid2SegIDMap[*it];
            bool isVaild = true;
            vector<int> segIndexs;
            if(segIds.size() == 3){
                map<int, int> segId2CoorIndex;
                int parentSegId;
                double minDist = 100000;
                //先找到离soma最近的点
                if(isSomaExists){
                    for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                        V_NeuronSWC seg = segments.seg[*segIt];
                        int index = getPointInSegIndex(*it, seg);
                        segId2CoorIndex[*segIt] = index;
                        if(index==0){
                            XYZ coor = seg.row[1];
                            if(distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z) < minDist){
                                minDist = distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z);
                                parentSegId = *segIt;
                            }
                        }
                        else if(index==seg.row.size()-1){
                            XYZ coor = seg.row[seg.row.size()-2];
                            if(distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z) < minDist){
                                minDist = distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z);
                                parentSegId = *segIt;
                            }
                        }
                    }

                    //调整线段走向
                    for(auto mapIt = segId2CoorIndex.begin(); mapIt != segId2CoorIndex.end(); mapIt++){
                        if(parentSegId == mapIt->first && mapIt->second != 0){
                            reverseSeg(segments.seg[mapIt->first]);
                        }
                        if(parentSegId != mapIt->first && mapIt->second != segments.seg[mapIt->first].row.size() - 1){
                            reverseSeg(segments.seg[mapIt->first]);
                        }
                    }
                }

                for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                    double length = getSegLength(segments.seg[*segIt]);
                    if(length < 20){
                        //                            it = bifurcationPoints.erase(it);
                        isVaild=false;
                        break;
                    }
                    int index = getPointInSegIndex(*it, segments.seg[*segIt]);
                    if(index == 0){
                        segIndexs.insert(segIndexs.begin(), *segIt);
                    }
                    else{
                        segIndexs.push_back(*segIt);
                    }
                }
            }
            else if(segIds.size() == 2){
                //先放parent, 再放chi
                int countNoParent=0;
                map<int, int> segId2CoorIndex;
                int parentSegId;
                XYZ minDistCoor;
                double minDist = 100000;
                //先找到离soma最近的点
                for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                    V_NeuronSWC seg = segments.seg[*segIt];
                    int index = getPointInSegIndex(*it, seg);
                    segId2CoorIndex[*segIt] = index;
                    if(index==0){
                        XYZ coor = seg.row[1];
                        if(distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z) < minDist){
                            minDist = distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z);
                            parentSegId = *segIt;
                            minDistCoor = coor;
                        }
                    }
                    else if(index==seg.row.size()-1){
                        XYZ coor = seg.row[seg.row.size()-2];
                        if(distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z) < minDist){
                            minDist = distance(coor.x, somaCoordinate.x, coor.y, somaCoordinate.y, coor.z, somaCoordinate.z);
                            parentSegId = *segIt;
                            minDistCoor = coor;
                        }
                    }
                    else{
                        XYZ coor1 = seg.row[index + 1];
                        XYZ coor2 = seg.row[index - 1];
                        if(distance(coor1.x, somaCoordinate.x, coor1.y, somaCoordinate.y, coor1.z, somaCoordinate.z) < minDist){
                            minDist = distance(coor1.x, somaCoordinate.x, coor1.y, somaCoordinate.y, coor1.z, somaCoordinate.z);
                            parentSegId = *segIt;
                            minDistCoor = coor1;
                        }
                        if(distance(coor2.x, somaCoordinate.x, coor2.y, somaCoordinate.y, coor2.z, somaCoordinate.z) < minDist){
                            minDist = distance(coor2.x, somaCoordinate.x, coor2.y, somaCoordinate.y, coor2.z, somaCoordinate.z);
                            parentSegId = *segIt;
                            minDistCoor = coor2;
                        }
                    }
                }

                //调整线段走向
                for(auto mapIt = segId2CoorIndex.begin(); mapIt != segId2CoorIndex.end(); mapIt++){
                    if(parentSegId == mapIt->first && minDistCoor != segments.seg[mapIt->first].row[mapIt->second + 1]){
                        reverseSeg(segments.seg[mapIt->first]);
                    }
                    if(parentSegId != mapIt->first && mapIt->second != segments.seg[mapIt->first].row.size() - 1){
                        reverseSeg(segments.seg[mapIt->first]);
                    }
                }

                for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                    double length = getSegLength(segments.seg[*segIt]);
                    if(length < 20){
                        //                            it = bifurcationPoints.erase(it);
                        isVaild=false;
                        break;
                    }
                    int index = getPointInSegIndex(*it, segments.seg[*segIt]);
                    if(index != segments.seg[*segIt].row.size() - 1){
                        segIndexs.insert(segIndexs.begin(), *segIt);
                    }
                    else{
                        segIndexs.push_back(*segIt);
                    }
                }
            }

            if(isVaild){
                if(segIndexs.size() == 2)
                    twoSegsMap[*it] = segIndexs;
                else{
                    threeSegsMap[*it] = segIndexs;
                }
                it++;
            }
            else{
                it = bifurcationPoints.erase(it);
            }
        }
    }

    for(auto it = threeSegsMap.begin(); it != threeSegsMap.end(); it++){
        XYZ curCoor;
        stringToXYZ(it->first, curCoor.x, curCoor.y, curCoor.z);
        XYZ parCoor = segments.seg[it->second[0]].row[1];
        vector<XYZ> chiCoors;
        for(auto chiSegIt=it->second.begin() + 1; chiSegIt!=it->second.end(); chiSegIt++){
            XYZ chiCoor;
            chiCoor.x = segments.seg[*chiSegIt].row[segments.seg[*chiSegIt].row.size() - 2].x;
            chiCoor.y = segments.seg[*chiSegIt].row[segments.seg[*chiSegIt].row.size() - 2].y;
            chiCoor.z = segments.seg[*chiSegIt].row[segments.seg[*chiSegIt].row.size() - 2].z;
            chiCoors.push_back(chiCoor);
        }
        XYZ parCoor_real = parCoor, chiCoor1_real = chiCoors[0], chiCoor2_real = chiCoors[1];

        float angle1, angle2;
        int count1 = 0;
        int count2 = 0;
        QVector3D vector1(parCoor_real.x-curCoor.x, parCoor_real.y-curCoor.y, parCoor_real.z-curCoor.z);

        for(auto coorIt = segments.seg[it->second[1]].row.rbegin(); coorIt != segments.seg[it->second[1]].row.rend(); coorIt++)
        {
            if(coorIt == segments.seg[it->second[1]].row.rbegin())
                continue;
            QVector3D vector2(coorIt->x-curCoor.x, coorIt->y-curCoor.y, coorIt->z-curCoor.z);
            angle1 += calculateAngleofVecs(vector1, vector2);
            count1++;
            if(count1 == 5){
                break;
            }
        }
        angle1 /= count1;

        for(auto coorIt = segments.seg[it->second[2]].row.rbegin(); coorIt != segments.seg[it->second[2]].row.rend(); coorIt++)
        {
            if(coorIt == segments.seg[it->second[2]].row.rbegin())
                continue;
            QVector3D vector3(coorIt->x-curCoor.x, coorIt->y-curCoor.y, coorIt->z-curCoor.z);
            angle2 += calculateAngleofVecs(vector1, vector3);
            count2++;
            if(count2 == 5){
                break;
            }
        }
        angle2 /= count2;

        if((angle1>0 && angle1<50) || (angle2>0 && angle2<50)){
            angleErrPoints.insert(it->first);
        }

    }

    for(auto it = twoSegsMap.begin(); it != twoSegsMap.end(); it++){
        XYZ curCoor;
        stringToXYZ(it->first, curCoor.x, curCoor.y, curCoor.z);
        int index = getPointInSegIndex(it->first, segments.seg[it->second[0]]);

        XYZ parCoor = segments.seg[it->second[0]].row[index + 1];
        vector<XYZ> chiCoors;
        chiCoors.push_back(segments.seg[it->second[0]].row[index - 1]);
        chiCoors.push_back(segments.seg[it->second[1]].row[segments.seg[it->second[1]].row.size() - 2]);

        XYZ parCoor_real = parCoor, chiCoor1_real = chiCoors[0], chiCoor2_real = chiCoors[1];

        float angle1, angle2;
        int count1 = 0;
        int count2 = 0;
        QVector3D vector1(parCoor_real.x-curCoor.x, parCoor_real.y-curCoor.y, parCoor_real.z-curCoor.z);

        for(auto coorIt = segments.seg[it->second[1]].row.rbegin(); coorIt != segments.seg[it->second[1]].row.rend(); coorIt++)
        {
            if(coorIt == segments.seg[it->second[1]].row.rbegin())
                continue;
            QVector3D vector2(coorIt->x-curCoor.x, coorIt->y-curCoor.y, coorIt->z-curCoor.z);
            angle1 += calculateAngleofVecs(vector1, vector2);
            count1++;
            if(count1 == 5){
                break;
            }
        }
        angle1 /= count1;

        for(int i = index - 1; i >= 0; i--)
        {
            XYZ coor = segments.seg[it->second[0]].row[i];
            QVector3D vector3(coor.x-curCoor.x, coor.y-curCoor.y, coor.z-curCoor.z);
            angle2 += calculateAngleofVecs(vector1, vector3);
            count2++;
            if(count2 == 5){
                break;
            }
        }
        angle2 /= count2;

        if((angle1>0 && angle1<50) || (angle2>0 && angle2<50)){
            angleErrPoints.insert(it->first);
        }
    }
    return angleErrPoints;
}

float calculateAngleofVecs(QVector3D vector1, QVector3D vector2){
    // 计算两个向量的夹角
    qreal dotProduct = QVector3D::dotProduct(vector1, vector2);
    qreal magnitude1 = vector1.length();
    qreal magnitude2 = vector2.length();

    qreal cosineAngle = dotProduct / (magnitude1 * magnitude2);
    // 计算弧度值
    qreal angleRadians = qAcos(cosineAngle);

    // 将弧度转换为角度
    qreal angleDegrees = qRadiansToDegrees(angleRadians);

    return angleDegrees;
}

//将soma点的半径设置为1.234
bool setSomaPointRadius(QString fileSaveName, V_NeuronSWC_list segments, XYZ somaCoordinate, double dist_thre, CollDetection* detectUtil, QString& msg){
    map<string, set<size_t> > wholeGrid2SegIDMap;
    map<string, bool> isEndPointMap;

    set<string> allPoints;
    map<string, set<string>> parentMap;
    map<string, set<string>> childMap;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        vector<int> rowN2Index(seg.row.size()+1);

        for(size_t j=0; j<seg.row.size(); ++j){
            rowN2Index[seg.row[j].n]=j;
        }

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2SegIDMap[gridKey].insert(size_t(i));
            allPoints.insert(gridKey);

            if(seg.row[j].parent!=-1){
                float x2Label=seg.row[rowN2Index[seg.row[j].parent]].x;
                float y2Label=seg.row[rowN2Index[seg.row[j].parent]].y;
                float z2Label=seg.row[rowN2Index[seg.row[j].parent]].z;
                QString parentKeyQ=QString::number(x2Label) + "_" + QString::number(y2Label) + "_" + QString::number(z2Label);
                string parentKey=parentKeyQ.toStdString();
                parentMap[gridKey].insert(parentKey);
                childMap[parentKey].insert(gridKey);
            }

            if(j == 0 || j == seg.row.size() - 1){
                isEndPointMap[gridKey] = true;
            }
        }
    }

    //末端点和分叉点
    vector<string> points;
    vector<set<int>> linksIndex;

    map<string,int> pointsIndexMap;

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(j==0 || j==seg.row.size()-1){
                //在pointsIndexMap中找不到某个线的末端点
                if(pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }else{
                if(wholeGrid2SegIDMap[gridKey].size()>1 &&
                    isEndPointMap.find(gridKey) != isEndPointMap.end() &&
                    pointsIndexMap.find(gridKey) == pointsIndexMap.end()){
                    points.push_back(gridKey);
                    linksIndex.push_back(set<int>());
                    //                    linksIndexVec.push_back(vector<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }
        }
    }
    qDebug()<<"points size: "<<points.size();

    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        vector<int> segIndexs;
        set<int> segIndexsSet;
        segIndexs.clear();
        segIndexsSet.clear();
        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(pointsIndexMap.find(gridKey) != pointsIndexMap.end()){
                int index = pointsIndexMap[gridKey];
                if(segIndexsSet.find(index) == segIndexsSet.end()){
                    segIndexs.push_back(index);
                    segIndexsSet.insert(index);
                }
            }
        }
        //        qDebug()<<"i : "<<i<<"seg size: "<<seg.row.size()<<" segIndexsSize: "<<segIndexs.size();
        for(size_t j=0; j<segIndexs.size()-1; ++j){
            if(segIndexs[j] == 1 || segIndexs[j+1] == 1){
                qDebug()<<segIndexs[j]<<" "<<segIndexs[j+1];
            }
            linksIndex[segIndexs[j]].insert(segIndexs[j+1]);
            //            linksIndexVec[segIndexs[j]].push_back(segIndexs[j+1]);
            linksIndex[segIndexs[j+1]].insert(segIndexs[j]);
            //            linksIndexVec[segIndexs[j+1]].push_back(segIndexs[j]);
        }
    }

    vector<NeuronSWC> outputSpecialPoints;

    for(size_t i=0; i<points.size(); ++i){
        //        qDebug()<<i<<" link size: "<<linksIndex[i].size();
        if(linksIndex[i].size() > 3){
            qDebug()<<i<<" link size: "<<linksIndex[i].size();
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            s.type = 8;
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y,
                         s.z, somaCoordinate.z) > dist_thre)
                outputSpecialPoints.push_back(s);
        }
    }

    vector<vector<size_t>> pairs;
    set<size_t> pset;

    size_t pre_tip_id=-1;
    size_t cur_tip_id=-1;

    double soma_radius=30;
    for(size_t i=0; i<points.size(); i++){
        if(linksIndex[i].size() == 3){
            pre_tip_id=cur_tip_id;
            cur_tip_id=i;
            if(pre_tip_id!=-1){
                NeuronSWC n1;
                stringToXYZ(points[pre_tip_id],n1.x,n1.y,n1.z);
                n1.type=6;
                NeuronSWC n2;
                stringToXYZ(points[cur_tip_id],n2.x,n2.y,n2.z);
                n2.type=6;
                set<size_t> n1Segs=wholeGrid2SegIDMap[points[pre_tip_id]];
                set<size_t> n2Segs=wholeGrid2SegIDMap[points[cur_tip_id]];
                int count1=0,count2=0;
                for(auto it1=n1Segs.begin();it1!=n1Segs.end();it1++)
                {
                    //                    qDebug()<<*it1;
                    //                    qDebug()<<getSegLength(inputSegList.seg[*it1]);
                    if(getSegLength(segments.seg[*it1])>40)
                        count1++;
                }

                for(auto it2=n2Segs.begin();it2!=n2Segs.end();it2++)
                {
                    //                    qDebug()<<*it2;
                    //                    qDebug()<<getSegLength(inputSegList.seg[*it2]);
                    if(getSegLength(segments.seg[*it2])>40)
                        count2++;
                }
                //                qDebug()<<"n2Segs end";
                if(!(count1>=2&&count2>=2)){
                    continue;
                }

                if(distance(n1.x,somaCoordinate.x,n1.y,somaCoordinate.y,n1.z,somaCoordinate.z)>soma_radius
                    &&distance(n2.x,somaCoordinate.x,n2.y,somaCoordinate.y,n2.z,somaCoordinate.z)>soma_radius){
                    double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                    if(distance((n1.x+n2.x)/2,somaCoordinate.x,(n1.y+n2.y)/2,somaCoordinate.y,(n1.z+n2.z)/2,somaCoordinate.z)>1e-7&&dist<dist_thre){
                        vector<size_t> v={pre_tip_id,cur_tip_id};
                        pairs.push_back(v);
                        pset.insert(pre_tip_id);
                        pset.insert(cur_tip_id);
                    }
                }
            }
        }
    }

    qDebug()<<pairs;
    //    qDebug()<<points;

    for(auto it=pset.begin(); it!=pset.end(); it++){
        qDebug()<<*it;
        NeuronSWC n;
        stringToXYZ(points[*it],n.x,n.y,n.z);
        n.type=6;
        outputSpecialPoints.push_back(n);
    }

    vector<NeuronSWC> bifurPoints;
    vector<NeuronSWC> mulfurPoints;

    for(int i=0;i<outputSpecialPoints.size();i++){
        if(outputSpecialPoints[i].type == 6)
            bifurPoints.push_back(outputSpecialPoints[i]);
        else if(outputSpecialPoints[i].type == 8)
            mulfurPoints.push_back(outputSpecialPoints[i]);
    }

    int count1=0;
    int count2=0;
    detectUtil->handleMulFurcation(mulfurPoints, count1);
    detectUtil->handleNearBifurcation(bifurPoints, count2);

    if(outputSpecialPoints.size()!=0){
        qDebug()<<"swc exists MulFurcation or NearBifurcation, notice the brown or yellow markers!";
        msg = "swc exists MulFurcation or NearBifurcation, notice the brown or yellow markers!";
        return false;
    }

    set<string> targetCoordSet;

    for(size_t i=0; i<points.size(); ++i){
        if(linksIndex[i].size() >= 3){
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)<dist_thre)
                targetCoordSet.insert(points[i]);
        }
    }

    if(targetCoordSet.size() == 0){
        for(size_t i=0; i<points.size(); ++i){
            if(linksIndex[i].size() == 2){
                NeuronSWC s;
                stringToXYZ(points[i],s.x,s.y,s.z);
                if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)<dist_thre)
                    targetCoordSet.insert(points[i]);
            }
        }
        if(targetCoordSet.size()==0){
            qDebug()<<"soma is not connected to one point!";
            msg = "soma is not connected to one point!";
            return false;
        }
    }

    bool isDeleteEnd = false;
    while(!isDeleteEnd){
        isDeleteEnd = true;
        for(int i=0; i<points.size(); ++i){
            if(linksIndex[i].size() == 1){
                int linkIndex = *(linksIndex[i].begin());
                linksIndex[i].clear();
                linksIndex[linkIndex].erase(std::find(linksIndex[linkIndex].begin(),linksIndex[linkIndex].end(),i));
                isDeleteEnd = false;
            }
        }
    }

    //检测3条及3条以上的边构成的环
    vector<string> newpoints;

    for(size_t i=0; i<points.size(); ++i){
        if(linksIndex[i].size()>=2)
            newpoints.push_back(points[i]);
    }

    set<string> specPoints;
    for(size_t i=0; i<newpoints.size(); ++i){
        specPoints.insert(newpoints[i]);
    }

    //检测2条边构成的环
    for(size_t i=0; i<segments.seg.size(); ++i){
        V_NeuronSWC seg = segments.seg[i];
        //        if(seg.row.size()<4)
        //            continue;
        float xLabel1 = seg.row[0].x;
        float yLabel1 = seg.row[0].y;
        float zLabel1 = seg.row[0].z;
        float xLabel2=seg.row[seg.row.size()-1].x;
        float yLabel2=seg.row[seg.row.size()-1].y;
        float zLabel2=seg.row[seg.row.size()-1].z;
        QString gridKeyQ1 = QString::number(xLabel1) + "_" + QString::number(yLabel1) + "_" + QString::number(zLabel1);
        string gridKey1 = gridKeyQ1.toStdString();
        QString gridKeyQ2 = QString::number(xLabel2) + "_" + QString::number(yLabel2) + "_" + QString::number(zLabel2);
        string gridKey2 = gridKeyQ2.toStdString();
        set<size_t> segSet1=wholeGrid2SegIDMap[gridKey1];
        set<size_t> segSet2=wholeGrid2SegIDMap[gridKey2];
        set<size_t> intersectionSet;
        set_intersection(segSet1.begin(),segSet1.end(),segSet2.begin(),segSet2.end(),inserter( intersectionSet , intersectionSet.begin()));

        if(intersectionSet.size()>=2){
            specPoints.insert(gridKey1);
            specPoints.insert(gridKey2);
        }
    }

    outputSpecialPoints.clear();
    int count =0;
    for(auto it=specPoints.begin(); it!=specPoints.end(); it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        s.type=0;
        outputSpecialPoints.push_back(s);
    }

    detectUtil->handleLoop(outputSpecialPoints, count);

    if(specPoints.size()!=0){
        qDebug()<<"swc exists loop, notice the white markers!";
        msg = "swc exists loop, notice the white markers!";
        return false;
    }

    double min_dist = 100;
    string somaCoord;
    for(auto it=targetCoordSet.begin(); it!=targetCoordSet.end(); it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        double dist = distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z);
        if(dist < min_dist)
        {
            min_dist = dist;
            somaCoord = *it;
        }
    }

    V_NeuronSWC seg = segments.seg[*wholeGrid2SegIDMap[somaCoord].begin()];
    for(int i=0; i<seg.row.size(); i++){
        float xLabel = seg.row[i].x;
        float yLabel = seg.row[i].y;
        float zLabel = seg.row[i].z;
        QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
        string gridKey = gridKeyQ.toStdString();
        if(gridKey == somaCoord){
            segments.seg[*wholeGrid2SegIDMap[somaCoord].begin()].row[i].r = 1.234;
            break;
        }
    }

    writeESWC_file(fileSaveName, V_NeuronSWC_list__2__NeuronTree(segments));
    return true;

}

int getSomaNumberFromSwcFile(QString filePath, float r, QString& msg){
    int number = -1;
    if (filePath.endsWith(".swc") || filePath.endsWith(".SWC") || filePath.endsWith(".eswc") || filePath.endsWith(".ESWC"))
    {
        QFile qf(filePath);
        QString arryRead;
        if(!qf.open(QIODevice::ReadOnly|QIODevice::Text)){
            msg = "cannot open swc file!";
            return -2;
        }
        arryRead=qf.readAll();

        qf.close();

        QStringList arryListWrite= arryRead.split("\n");
        //        for(int i=0;i<arryListWrite.size();i++){
        //            qDebug()<<arryListWrite.at(i);
        //        }

        // QIODevice::Text:以文本方式打开文件，读取时“\n”被自动翻译为换行符，写入时字符串结束符会自动翻译为系统平台的编码，如 Windows 平台下是“\r\n”
        if (!qf.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            msg = "cannot open swc file!";
            return -2;
        }
        QTextStream streamWrite(&qf);
        for(int i=0;i<arryListWrite.size()-1;i++){      //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
            if(arryListWrite.at(i).contains("#")){
                streamWrite<<arryListWrite.at(i)<<"\n";
            }else{
                QString contentWrite= arryListWrite.at(i);
                QStringList swcInfo=contentWrite.split(' ',Qt::SkipEmptyParts);
                if(swcInfo[5] == QString::number(r))
                {
                    number = swcInfo[0].toInt();
                    swcInfo[5] = "1.000";
                }
                contentWrite=swcInfo.join(' ');
                streamWrite<<contentWrite<<"\n";
            }
        }
        qf.close();
        return number;
    }

}
