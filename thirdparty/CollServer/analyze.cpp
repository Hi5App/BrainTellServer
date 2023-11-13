#include "analyze.h"

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
        }
    }

    counts.push_back(biCount);
    counts.push_back(mulCount);
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

set<string> getAngleErrPoints(float dist_thre, XYZ somaCoordinate, V_NeuronSWC_list segments){
    set<string> angleErrPoints;

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
            if(distance(s.x, somaCoordinate.x, s.y, somaCoordinate.y, s.z, somaCoordinate.z)>dist_thre)
                bifurcationPoints.insert(points[i]);
        }
    }

    for(auto it=bifurcationPoints.begin(); it!=bifurcationPoints.end();){
        set<int> types = point2TypeMap[*it];

        if (types.size()!=1) {
            it = bifurcationPoints.erase(it); // 通过迭代器去除元素，并返回下一个有效迭代器
        }
        else if(*types.begin()!=3){
            it = bifurcationPoints.erase(it);
        }
        else {
            if(parentMap.find(*it)!=parentMap.end() && parentMap[*it].size()==1
                && childMap.find(*it)!=childMap.end() && childMap[*it].size()==2)
            {
                set<size_t> segIds = wholeGrid2SegIDMap[*it];
                bool flag = true;
                if(segIds.size() == 3){
                    int countNoParent=0;
                    int countNoChild=0;
                    for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                        V_NeuronSWC seg = segments.seg[*segIt];
                        int index = getPointInSegIndex(*it, seg);

                        if(index==0)
                            countNoChild++;
                        if(index==seg.row.size()-1)
                            countNoParent++;

                        double length = getSegLength(segments.seg[*segIt]);
                        if(length < 20){
                            it = bifurcationPoints.erase(it);
                            flag=false;
                            break;
                        }
                    }

                    if(!(countNoChild==1 && countNoParent==2))
                        flag=false;
                }
                else if(segIds.size() == 2){
                    int countNoParent=0;
                    for(auto segIt=segIds.begin(); segIt!=segIds.end(); segIt++){
                        V_NeuronSWC seg = segments.seg[*segIt];
                        int index = getPointInSegIndex(*it, seg);

                        if(index==seg.row.size()-1)
                            countNoParent++;

                        double length = getPartOfSegLength(seg, index);
                        if(length < 20){
                            it = bifurcationPoints.erase(it);
                            flag=false;
                            break;
                        }
                    }

                    if(countNoParent!=1)
                        flag=false;
                }

                if(flag)
                    it++;
            }
            else
            {
                it = bifurcationPoints.erase(it);
            }
        }
    }

    for(auto it=bifurcationPoints.begin(); it!=bifurcationPoints.end(); it++){
        XYZ curCoor;
        stringToXYZ(*it, curCoor.x, curCoor.y, curCoor.z);
        XYZ parCoor;
        stringToXYZ(*parentMap[*it].begin(), parCoor.x, parCoor.y, parCoor.z);
        vector<XYZ> chiCoors;
        for(auto chiIt=childMap[*it].begin(); chiIt!=childMap[*it].end(); chiIt++){
            XYZ chiCoor;
            stringToXYZ(*chiIt, chiCoor.x, chiCoor.y, chiCoor.z);
            chiCoors.push_back(chiCoor);
        }
        QVector3D vector1(parCoor.x-curCoor.x, parCoor.y-curCoor.y, parCoor.z-curCoor.z);
        QVector3D vector2(chiCoors[0].x-curCoor.x, chiCoors[0].y-curCoor.y, chiCoors[0].z-curCoor.z);
        QVector3D vector3(chiCoors[1].x-curCoor.x, chiCoors[1].y-curCoor.y, chiCoors[1].z-curCoor.z);

        float angle1 = calculateAngleofVecs(vector1, vector2);
        float angle2 = calculateAngleofVecs(vector1, vector3);
        if((angle1>0 && angle1<35) || (angle2>0 && angle2<35)){
            angleErrPoints.insert(*it);
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
