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
        bool flag1=true;
        bool flag2=true;
        string savedgridKey;

        for(size_t j=0; j<seg.row.size(); ++j){
            if(j!=0 && j!=seg.row.size()-1){
                continue;
            }
            else{
                float xLabel = seg.row[j].x;
                float yLabel = seg.row[j].y;
                float zLabel = seg.row[j].z;
                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
                string gridKey = gridKeyQ.toStdString();
                int size=wholeGrid2SegIDMap[gridKey].size();
                if(j==0 && size<=1){
                    flag1=false;
                    savedgridKey=gridKey;
                }
                if(j==seg.row.size()-1 && size<=1){
                    flag2=false;
                }
            }
        }

        if(!flag1 && !flag2){
            dissociativePoints.insert(savedgridKey);
        }
    }

    return dissociativePoints;
}
