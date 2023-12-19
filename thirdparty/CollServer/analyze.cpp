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
//        qDebug()<<"types: "<<point2TypeMap[*it].size();
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
//                            it = bifurcationPoints.erase(it);
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
                            flag=false;
                            break;
                        }
                    }

                    if(countNoParent!=1)
                        flag=false;
                }

                if(flag)
                    it++;
                else{
                    it = bifurcationPoints.erase(it);
                }
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

//将soma点的半径设置为1.234
bool setSomaPointRadius(QString fileSaveName, V_NeuronSWC_list segments, XYZ somaCoordinate, double dist_thre, CollDetection* detectUtil, QString& msg){
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

    vector<NeuronSWC> outputSpecialPoints;
    int count =0;
    for(auto it=specPoints.begin(); it!=specPoints.end(); it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        s.type=0;
        outputSpecialPoints.push_back(s);
    }

    detectUtil->handleLoop(outputSpecialPoints, count);

    if(specPoints.size()!=0){
        qDebug()<<"swc exists loop,notice the white markers!";
        msg = "swc exists loop,notice the white markers!";
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
