#include "colldetection.h"
#include "coll_server.h"
#include "sort_swc.h"

XYZ CollDetection::somaCoordinate;
bool CollDetection::isSomaExists;

CollDetection::CollDetection(CollServer* curServer, QObject* parent):myServer(static_cast<CollServer*>(parent)){
    myServer=curServer;
    accessManager=new QNetworkAccessManager(this);
    HostAddress="http://114.117.165.134:26000/SuperUser";
}

XYZ CollDetection::getSomaCoordinate(QString apoPath){
    isSomaExists=false;
    XYZ coordinate;
    coordinate.x=-1;
    coordinate.y=-1;
    coordinate.z=-1;
    QFile qf(apoPath);
    if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"apofile open error";
        return coordinate;
    }
    char *buf;
    char _buf[1000];
    qf.readLine(_buf, sizeof(_buf));
    for (buf=_buf; (*buf && *buf==' '); buf++);
    if (buf[0]=='#' ||buf[0]=='\0')
    {
        if(!qf.atEnd())
        {
            qf.readLine(_buf, sizeof(_buf));
            for (buf=_buf; (*buf && *buf==' '); buf++);
        }
        else{
            qDebug()<<"apofile format error";
            return coordinate;
        }
    }
    else{
        qDebug()<<"apofile format error";
        return coordinate;
    }
    QStringList qsl = QString(buf).split(",");
    if (qsl.size()==0){
        qDebug()<<"apofile format error";
        return coordinate;
    }
    else{
        for (int i=4; i<qsl.size(); i++)
        {
            qsl[i].truncate(200); //change from 99 to 200, 20121212, by PHC
            if (i==4) coordinate.z = qsl[i].toFloat();
            if (i==5) coordinate.x = qsl[i].toFloat();
            if (i==6)
            {
                coordinate.y = qsl[i].toFloat();
                isSomaExists=true;
                break;
            }
        }
    }
    return coordinate;
}

void CollDetection::detectTips(){
    vector<NeuronSWC> tipPoints=tipDetection(myServer->segments,30);
    myServer->imediateSave();
    handleTip(tipPoints);
}

void CollDetection::detectCrossings(){
    map<string, vector<string>> parentsDict;
    map<string, vector<string>> offspringsDict;
    vector<vector<NeuronSWC>> crossingPoints=crossingDetection(myServer->segments, parentsDict, offspringsDict);
    myServer->imediateSave();
    handleCrossing(crossingPoints, parentsDict, offspringsDict);
}

void CollDetection::detectOthers(){
    int count=0;
    vector<NeuronSWC> outputSpecialPoints = specStructsDetection(myServer->segments);
    vector<NeuronSWC> bifurPoints;
    vector<NeuronSWC> mulfurPoints;
    vector<NeuronSWC> loopPoints;

    for(int i=0;i<outputSpecialPoints.size();i++){
        if(outputSpecialPoints[i].type == 6)
            bifurPoints.push_back(outputSpecialPoints[i]);
        else if(outputSpecialPoints[i].type == 0)
            loopPoints.push_back(outputSpecialPoints[i]);
        else if(outputSpecialPoints[i].type == 8)
            mulfurPoints.push_back(outputSpecialPoints[i]);
    }

    handleMulFurcation(mulfurPoints, count);
    handleLoop(loopPoints, count);
    handleNearBifurcation(bifurPoints, count);

    if(count!=0){
        myServer->imediateSave();
    }
}

vector<NeuronSWC> CollDetection::specStructsDetection(V_NeuronSWC_list inputSegList, double dist_thresh){

    vector<NeuronSWC> outputSpecialPoints;
    outputSpecialPoints.clear();

    map<string, set<size_t> > wholeGrid2segIDmap;
    map<string, bool> isEndPointMap;
    map<string, set<string>> parentMap;

    set<string> allPoints;
    map<string, set<string>> childMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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
            wholeGrid2segIDmap[gridKey].insert(size_t(i));
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

    //    for(map<string, set<size_t> >::iterator it = wholeGrid2segIDmap.begin(); it != wholeGrid2segIDmap.end(); ++it){
    //        if(it->second.size() > 5){
    //            qDebug()<<it->first.c_str()<<" "<<it->second.size();
    //        }

    //    }


    //末端点和分叉点
    vector<string> points;
    vector<set<int>> linksIndex;
    //    vector<vector<int>> linksIndexVec;
    map<string,int> pointsIndexMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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
                if(wholeGrid2segIDmap[gridKey].size()>1 &&
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

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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

    for(size_t i=0; i<points.size(); ++i){
//        qDebug()<<i<<" link size: "<<linksIndex[i].size();
        if(linksIndex[i].size() > 3){
            qDebug()<<i<<" link size: "<<linksIndex[i].size();
            NeuronSWC s;
            stringToXYZ(points[i],s.x,s.y,s.z);
            s.type = 8;
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
                set<size_t> n1Segs=wholeGrid2segIDmap[points[pre_tip_id]];
                set<size_t> n2Segs=wholeGrid2segIDmap[points[cur_tip_id]];
                int count1=0,count2=0;
                for(auto it1=n1Segs.begin();it1!=n1Segs.end();it1++)
                {
                    qDebug()<<*it1;
                    qDebug()<<getSegLength(inputSegList.seg[*it1]);
                    if(getSegLength(inputSegList.seg[*it1])>40)
                        count1++;
                }
                qDebug()<<"n1Segs end";
                for(auto it2=n2Segs.begin();it2!=n2Segs.end();it2++)
                {
                    qDebug()<<*it2;
                    qDebug()<<getSegLength(inputSegList.seg[*it2]);
                    if(getSegLength(inputSegList.seg[*it2])>40)
                        count2++;
                }
                qDebug()<<"n2Segs end";
                if(!(count1>=2&&count2>=2)){
                    continue;
                }
                if(isSomaExists){
                    if(distance(n1.x,somaCoordinate.x,n1.y,somaCoordinate.y,n1.z,somaCoordinate.z)>soma_radius
                        &&distance(n2.x,somaCoordinate.x,n2.y,somaCoordinate.y,n2.z,somaCoordinate.z)>soma_radius){
                        double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                        if(distance((n1.x+n2.x)/2,somaCoordinate.x,(n1.y+n2.y)/2,somaCoordinate.y,(n1.z+n2.z)/2,somaCoordinate.z)>1e-7&&dist<dist_thresh){
                            vector<size_t> v={pre_tip_id,cur_tip_id};
                            pairs.push_back(v);
                            pset.insert(pre_tip_id);
                            pset.insert(cur_tip_id);
                        }
                    }
                }
                else{
                    double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                    if(dist<dist_thresh){
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

    size_t start=0;
    for(size_t i=0; i<newpoints.size(); ++i){
        qDebug()<<QString::fromStdString(newpoints[i])<<" "<<parentMap[newpoints[i]].size();
        /*if(newLinksIndexVec[i].size()>=2&&counts[i]>=3&&newLinksIndexVec[i].size()!=counts[i])*/
        if(parentMap[newpoints[i]].size()>=2){
            size_t interval=i-start;
            int nums=interval/12;
            for(int j=0;j<nums;j++){
                NeuronSWC s;
                stringToXYZ(newpoints[start+(j+1)*12],s.x,s.y,s.z);
                s.type = 0;
                outputSpecialPoints.push_back(s);
            }

            NeuronSWC s;
            stringToXYZ(newpoints[i],s.x,s.y,s.z);
            s.type = 0;
            outputSpecialPoints.push_back(s);

            start=i+1;
            qDebug()<<"loop exists";
        }

    }

    if(start<newpoints.size()){
        size_t interval=newpoints.size()-1-start;
        int nums=interval/12;
        for(int j=0;j<nums;j++){
            NeuronSWC s;
            stringToXYZ(newpoints[start+(j+1)*12],s.x,s.y,s.z);
            s.type = 0;
            outputSpecialPoints.push_back(s);
        }
    }

    //检测2条边构成的环
    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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
        set<size_t> segSet1=wholeGrid2segIDmap[gridKey1];
        set<size_t> segSet2=wholeGrid2segIDmap[gridKey2];
        set<size_t> intersectionSet;
        set_intersection(segSet1.begin(),segSet1.end(),segSet2.begin(),segSet2.end(),inserter( intersectionSet , intersectionSet.begin() ));
        if(intersectionSet.size()>=2){
            NeuronSWC s;
            stringToXYZ(gridKey1,s.x,s.y,s.z);
            s.type = 0;
            outputSpecialPoints.push_back(s);
            stringToXYZ(gridKey2,s.x,s.y,s.z);
            outputSpecialPoints.push_back(s);
        }
    }

    return outputSpecialPoints;

}

vector<NeuronSWC> CollDetection::tipDetection(V_NeuronSWC_list inputSegList, double dist_thresh){
    vector<NeuronSWC> outputSpecialPoints;
    outputSpecialPoints.clear();
    map<string, bool> isEndPointMap;
    map<string, set<size_t> > wholeGrid2segIDmap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];

        for(size_t j=0; j<seg.row.size(); ++j){
            float xLabel = seg.row[j].x;
            float yLabel = seg.row[j].y;
            float zLabel = seg.row[j].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            wholeGrid2segIDmap[gridKey].insert(size_t(i));

            if(j == 0 || j == seg.row.size() - 1){
                isEndPointMap[gridKey] = true;
            }
        }
    }

    //末端点和分叉点
    vector<string> points;
    vector<set<int>> linksIndex;
    //    vector<vector<int>> linksIndexVec;
    map<string,int> pointsIndexMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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
                if(wholeGrid2segIDmap[gridKey].size()>1 &&
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

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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

    //detect tips
    set<string> tips;

    for(int i=0;i<inputSegList.seg.size();i++){
        V_NeuronSWC seg = inputSegList.seg[i];
        float xLabel1 = seg.row[0].x;
        float yLabel1 = seg.row[0].y;
        float zLabel1 = seg.row[0].z;
        QString gridKeyQ1 = QString::number(xLabel1) + "_" + QString::number(yLabel1) + "_" + QString::number(zLabel1);
        string gridKey1 = gridKeyQ1.toStdString();
        float xLabel2 = seg.row[seg.row.size()-1].x;
        float yLabel2 = seg.row[seg.row.size()-1].y;
        float zLabel2 = seg.row[seg.row.size()-1].z;
        QString gridKeyQ2 = QString::number(xLabel2) + "_" + QString::number(yLabel2) + "_" + QString::number(zLabel2);
        string gridKey2 = gridKeyQ2.toStdString();
        if(wholeGrid2segIDmap[gridKey1].size()==1&&wholeGrid2segIDmap[gridKey2].size()>1)
        {
            if(isSomaExists&&sqrt((xLabel1-somaCoordinate.x)*(xLabel1-somaCoordinate.x)+
                (yLabel1-somaCoordinate.y)*(yLabel1-somaCoordinate.y)+(zLabel1-somaCoordinate.z)*(zLabel1-somaCoordinate.z))>50)
                tips.insert(gridKey1);
        }
        if(wholeGrid2segIDmap[gridKey2].size()==1&&wholeGrid2segIDmap[gridKey1].size()>1)
        {
            if(isSomaExists&&sqrt((xLabel2-somaCoordinate.x)*(xLabel2-somaCoordinate.x)+
                                     (yLabel2-somaCoordinate.y)*(yLabel2-somaCoordinate.y)+(zLabel2-somaCoordinate.z)*(zLabel2-somaCoordinate.z))>50)
                tips.insert(gridKey2);
        }
    }

    for(auto it=tips.begin();it!=tips.end();it++){
        vector<size_t> visitedSegIds;
        size_t segId=*wholeGrid2segIDmap[*it].begin();
        visitedSegIds.push_back(segId);
        V_NeuronSWC seg = inputSegList.seg[segId];
        float xLabel0 = seg.row[0].x;
        float yLabel0 = seg.row[0].y;
        float zLabel0 = seg.row[0].z;
        QString gridKeyQ0 = QString::number(xLabel0) + "_" + QString::number(yLabel0) + "_" + QString::number(zLabel0);
        string gridKey0 = gridKeyQ0.toStdString();
        float tipBranchLength=0;
        bool isReverse=false;
        if(wholeGrid2segIDmap[gridKey0].size()!=1)
        {
            isReverse=true;
        }
        bool flag=true;
        while(true){
            int size=seg.row.size();
            vector<int> indexs(size);
            for(int m=0;m<size;m++)
                indexs[m]=m;
            if(isReverse)
                reverse(indexs.begin(),indexs.end());
            for(int i=0;i<size;i++){
                int index=indexs[i];
                float xLabel = seg.row[index].x;
                float yLabel = seg.row[index].y;
                float zLabel = seg.row[index].z;
                QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
                string gridKey = gridKeyQ.toStdString();
                vector<string>::iterator it2=find(points.begin(),points.end(),gridKey);
                if(it2!=points.end()){
                    int index2=it2-points.begin();
                    if(linksIndex[index2].size()>=3){
                        flag=false;
                        break;
                    }
                    else{
                        if(index==seg.row.size()-1)
                            break;
                        tipBranchLength+=distance(xLabel,seg.row[index+1].x,
                                                    yLabel,seg.row[index+1].y,
                                                    zLabel,seg.row[index+1].z);
                        if(tipBranchLength>=dist_thresh)
                            break;
                        continue;
                    }
                }
                tipBranchLength+=distance(xLabel,seg.row[index+1].x,
                                            yLabel,seg.row[index+1].y,
                                            zLabel,seg.row[index+1].z);
                if(tipBranchLength>=dist_thresh)
                    break;
            }

            if(tipBranchLength>=dist_thresh||!flag)
                break;
            float xLabel = seg.row[indexs[size-1]].x;
            float yLabel = seg.row[indexs[size-1]].y;
            float zLabel = seg.row[indexs[size-1]].z;
            QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
            string gridKey = gridKeyQ.toStdString();
            if(wholeGrid2segIDmap[gridKey].size()!=2)
            {
                tipBranchLength=0;
                break;
            }
            if(segId!=*wholeGrid2segIDmap[gridKey].begin())
                segId=*wholeGrid2segIDmap[gridKey].begin();
            else
                segId=*wholeGrid2segIDmap[gridKey].begin()+1;
            if(find(visitedSegIds.begin(),visitedSegIds.end(),segId)==visitedSegIds.end())
                visitedSegIds.push_back(segId);
            else
            {
                tipBranchLength=0;
                break;
            }
            seg = inputSegList.seg[segId];
            float xLabel2 = seg.row[0].x;
            float yLabel2 = seg.row[0].y;
            float zLabel2 = seg.row[0].z;
            QString gridKeyQ2 = QString::number(xLabel2) + "_" + QString::number(yLabel2) + "_" + QString::number(zLabel2);
            string gridKey2 = gridKeyQ2.toStdString();
            if(gridKey2!=gridKey)
                isReverse=true;
            else
                isReverse=false;
        }
        if(tipBranchLength>=dist_thresh){
            NeuronSWC s;
            stringToXYZ(*it,s.x,s.y,s.z);
            s.type = 10;
            outputSpecialPoints.push_back(s);
        }

    }

    return outputSpecialPoints;
}

vector<vector<NeuronSWC>> CollDetection::crossingDetection(V_NeuronSWC_list inputSegList, map<string, vector<string>> &parentsDict, map<string, vector<string>> &offspringsDict){
    vector<vector<NeuronSWC>> outputSpecialPoints;
    map<string, set<string>> parentMap;

    set<string> allPoints;
    map<string, set<string>> childMap;

    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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

        }
    }

    //get_soma_nearby_points
    double ignore_radius_from_soma = 50;
    set<string> remain_pts;
    for(auto it=allPoints.begin();it!=allPoints.end();it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        if(distance(s.x,somaCoordinate.x,s.y,somaCoordinate.y,s.z,somaCoordinate.z)>ignore_radius_from_soma)
            remain_pts.insert(*it);
    }

    //get_linkages_with_thresh
    int offspring_thresh=10;
    for(auto it=allPoints.begin();it!=allPoints.end();it++){
        string gridKey=*it;
        int os_id=0;
        set<string> cur_set;
        vector<string> cur_vector;
        while(os_id<offspring_thresh){
            if(parentMap.find(gridKey)==parentMap.end()){
                break;
            }
            if(parentMap[gridKey].size()==1){
                bool result=cur_set.insert(*parentMap[gridKey].begin()).second;
                if(!result)
                    break;
                cur_vector.push_back(*parentMap[gridKey].begin());
                gridKey=*parentMap[gridKey].begin();
                os_id+=1;
            }
            if(parentMap[gridKey].size()>=2){
                cur_set.clear();
                cur_vector.clear();
                break;
            }
        }
        parentsDict[*it]=cur_vector;
        gridKey=*it;
        os_id=0;
        cur_set.clear();
        cur_vector.clear();
        while(os_id<offspring_thresh){
            if(childMap.find(gridKey)==childMap.end()){
                break;
            }
            if(childMap[gridKey].size()==1){
                bool result=cur_set.insert(*childMap[gridKey].begin()).second;
                if(!result)
                    break;
                cur_vector.push_back(*childMap[gridKey].begin());
                gridKey=*childMap[gridKey].begin();
                os_id+=1;
            }
            if(childMap[gridKey].size()>=2){
                cur_set.clear();
                cur_vector.clear();
                break;
            }
        }
        offspringsDict[*it]=cur_vector;
    }
//    for(auto it : parentsDict){
//        for(auto p_coor: it.second){
//            if(offspringsDict.find(p_coor)==offspringsDict.end()){
//                offspringsDict[p_coor]=set<string>();
//                offspringsDict[p_coor].insert(it.first);
//            }
//            else{
//                offspringsDict[p_coor].insert(it.first);
//            }
//        }
//    }

    //calc_pairwise_dist
    set<string> d1_coord_set;
    set<set<string>> dmin_coord_listAll ;
    for(auto it=remain_pts.begin();it!=remain_pts.end();it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        set<string> cur_set(parentsDict[*it].begin(),parentsDict[*it].end());
        if(cur_set.size()<5)
            continue;
        cur_set.insert(*it);
        if(offspringsDict.find(*it)!=offspringsDict.end()){
            if(offspringsDict[*it].size()<5)
                continue;
            set<string> offSet(offspringsDict[*it].begin(),offspringsDict[*it].end());
            set_union(cur_set.begin(), cur_set.end(), offSet.begin(), offSet.end(), inserter(cur_set,cur_set.begin()));
            set<string> pts;
            set_difference(allPoints.begin(),allPoints.end(),cur_set.begin(),cur_set.end(), inserter(pts, pts.begin()));
            set<string> nearPoints;
            for(auto coorIt=pts.begin();coorIt!=pts.end();coorIt++){
                NeuronSWC tmp;
                stringToXYZ(*coorIt,tmp.x,tmp.y,tmp.z);
                if(fabs(s.x-tmp.x)<5&&fabs(s.y-tmp.y)&&fabs(s.z-tmp.z)<5){
                    nearPoints.insert(*coorIt);
                }
            }
            double cur_dmin=2;
            string targetCoor="";
            for(auto coorIt=nearPoints.begin();coorIt!=nearPoints.end();coorIt++){
                NeuronSWC tmp;
                stringToXYZ(*coorIt,tmp.x,tmp.y,tmp.z);
                double dist=distance(s.x,tmp.x,s.y,tmp.y,s.z,tmp.z);
                if(dist<=cur_dmin){
                    bool hasCommonParents=false;
                    for(auto pr0=parentsDict[*it].begin();pr0!=parentsDict[*it].end();pr0++){
                        for(auto pr1=parentsDict[*coorIt].begin();pr1!=parentsDict[*coorIt].end();pr1++){
                            if(*pr0==*pr1){
                                hasCommonParents=true;
                                break;
                            }
                        }
                        if(hasCommonParents==true)
                            break;
                    }
                    if(!hasCommonParents){
                        cur_dmin=dist;
                        targetCoor=*coorIt;
                    }
                }
            }
            if(targetCoor!=""){
                d1_coord_set.insert(targetCoor);
                d1_coord_set.insert(*it);
                dmin_coord_listAll.insert(d1_coord_set);
                d1_coord_set.clear();
            }
        }else {
            continue;
        }
    }

    for(auto it=dmin_coord_listAll.begin();it!=dmin_coord_listAll.end();it++)
    {
        vector<NeuronSWC> tmpVec;
        NeuronSWC s;
        s.type=15;
        for(auto coorIt=(*it).begin();coorIt!=(*it).end();coorIt++){
            stringToXYZ(*coorIt,s.x,s.y,s.z);
            tmpVec.push_back(s);
        }
        outputSpecialPoints.push_back(tmpVec);
    }
    return outputSpecialPoints;
}

void CollDetection::handleMulFurcation(vector<NeuronSWC>& outputSpecialPoints, int& count){
    for(int i=0;i<outputSpecialPoints.size();i++){
        if(isSomaExists)
        {
            if((abs(outputSpecialPoints[i].x - somaCoordinate.x) > 5 ||
                 abs(outputSpecialPoints[i].y - somaCoordinate.y) > 5  ||
                 abs(outputSpecialPoints[i].z - somaCoordinate.z) > 5 ))
            {
                count++;
                QStringList result;
                result.push_back(QString("%1 server").arg(0));
                result.push_back(QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z));
                QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
                //            const std::string data=msg.toStdString();
                //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();

                //emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size());
                myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
                qDebug()<<"Server finish /WARN_MulBifurcation";

                //                for(auto &socket:sockets){
                //                    socket->sendmsgs({msg});
                //                }
                emit myServer->clientSendMsgs({msg});
            }
        }

        else{
            count++;
            QStringList result;
            result.push_back(QString("%1 server").arg(0));
            result.push_back(QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z));
            QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
            //            const std::string data=msg.toStdString();
            //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
            //                auto sockets=hashmap.values();
            //                emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
            myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
            qDebug()<<"Server finish /WARN_MulBifurcation";

            //                for(auto &socket:sockets){
            //                    socket->sendmsgs({msg});
            //                }
            emit myServer->clientSendMsgs({msg});
        }
    }
}

void CollDetection::handleLoop(vector<NeuronSWC>& outputSpecialPoints, int& count){
    for(int i=0;i<outputSpecialPoints.size();i++){
        count++;
        QStringList result;
        result.push_back(QString("%1 server").arg(0));
        result.push_back(QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z));
        QString msg=QString("/WARN_Loop:"+result.join(","));
        //            auto sockets=hashmap.values();
        //            emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_Loop:").size()));
        myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_Loop:").size()));
        qDebug()<<"Server finish /WARN_Loop";

        //            for(auto &socket:sockets){
        //                socket->sendmsgs({msg});
        //            }
        emit myServer->clientSendMsgs({msg});
    }
}

void CollDetection::handleNearBifurcation(vector<NeuronSWC>& bifurPoints, int& count){
    count+=bifurPoints.size();
    for(int i=0;i<bifurPoints.size();i++){
        QStringList result;
        result.push_back(QString("%1 server").arg(0));
        result.push_back(QString("%1 %2 %3 %4").arg(bifurPoints[i].type).arg(bifurPoints[i].x).arg(bifurPoints[i].y).arg(bifurPoints[i].z));
        QString msg=QString("/WARN_NearBifurcation:"+result.join(","));
        //            const std::string data=msg.toStdString();
        //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
        //            auto sockets=hashmap.values();
        //        emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_Crossing:").size()));
        myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_NearBifurcation:").size()));
        qDebug()<<"Server finish /WARN_NearBifurcation";
        //                for(auto &socket:sockets){
        //                    socket->sendmsgs({msg});
        //                }
        emit myServer->clientSendMsgs({msg});
    }

}

void CollDetection::handleTip(vector<NeuronSWC>& tipPoints){
//    for(int i=0;i<tipPoints.size();i++){
//        QStringList result;
//        result.push_back(QString("%1 server").arg(0));
//        result.push_back(QString("%1 %2 %3 %4").arg(tipPoints[i].type).arg(tipPoints[i].x).arg(tipPoints[i].y).arg(tipPoints[i].z));
//        QString msg=QString("/WARN_Tip:"+result.join(","));
//        myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_Tip:").size()));
//        qDebug()<<"Server finish /WARN_Tip";
//        emit myServer->clientSendMsgs({msg});
//    }

    if(tipPoints.size()!=0){
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        QString swcFileName=myServer->getAnoName()+".ano.eswc";
        QString fileSaveName=myServer->swcpath.left(myServer->swcpath.size()-QString(".ano.eswc").size())+"_sorted.ano.eswc";

        sortSWC(myServer->swcpath,fileSaveName,0);

        // 创建一个QFile对象，用于读取要上传的文件
        QFile *file = new QFile(fileSaveName);
        file->open(QIODevice::Text|QIODevice::ReadWrite);
        file->setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
        setSWCRadius(fileSaveName,1);

        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"swcFile\"; filename=\""+ swcFileName + "\"")); // file为后端定义的key，filename即为excel文件名
        filePart.setBodyDevice(file);
        file->setParent(multiPart); // 文件将由multiPart对象进行管理

        multiPart->append(filePart);
        QNetworkRequest fileRequest(QUrl(HostAddress+"/detect/file/for-missing"));
        // 发送HTTP POST请求
        QNetworkReply* fileReply = accessManager->post(fileRequest, multiPart);
        multiPart->setParent(fileReply); // reply对象将负责删除multiPart对象
        QEventLoop tmpeventLoop;
        connect(fileReply, &QNetworkReply::finished, &tmpeventLoop, &QEventLoop::quit);
        tmpeventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(fileReply->error())
        {
            qDebug() << "SENDFILEERROR!";
            qDebug() << fileReply->errorString();
        }
        int fileResCode=fileReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"sendFile"<<fileResCode;
        //清理资源
        QFile::remove(fileSaveName);
        file->close();
        fileReply->deleteLater();

        QJsonObject json;
        QString obj=myServer->getImage();
        json.insert("obj",obj);
        json.insert("res", myServer->RES);
        QJsonArray coorList;
        for(int i=0; i<tipPoints.size();i++){
            QJsonObject coor;
            coor.insert("x", tipPoints[i].x);
            coor.insert("y", tipPoints[i].y);
            coor.insert("z", tipPoints[i].z);
            coorList.append(coor);
        }
        json.insert("coors",coorList);

        //        QFile *file = new QFile(swcpath);
        //        file->open(QIODevice::ReadOnly | QIODevice::Text);
        //        QString content = file->readAll();
        //        QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8());
        //        QJsonObject fileObject = jsonDocument.object();
        //        json.insert("swcFile",fileObject);

        // 创建一个QHttpMultiPart对象，用于构建multipart/form-data请求
        //        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // 创建一个QFile对象，用于读取要上传的文件
        //        QFile *file = new QFile(swcpath);
        //        file->open(QIODevice::ReadOnly | QIODevice::Text);


        // 将文件添加到multipart请求中
        //        QHttpPart filePart;
        //        QString swcFileName=AnoName+".ano.eswc";
        //        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"swcFile\"; filename=\"%1\"").arg(swcFileName)));
        //        filePart.setBodyDevice(file);
        //        file->setParent(multiPart); // 文件将由multiPart对象进行管理

        //        multiPart->append(filePart);
        //        QHttpPart jsonPart;
        //        jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"info\""));
        //        jsonPart.setBody(QJsonDocument(json).toJson());

        //        multiPart->append(jsonPart);

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray=str.toUtf8();

        // 创建一个QNetworkRequest对象，设置URL和请求方法
        QNetworkRequest request(QUrl(HostAddress+"/detect/missing"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        //        request.setRawHeader("Content-Type", "multipart/form-data; boundary=" + multiPart->boundary());

        // 发送HTTP POST请求
        QNetworkReply* reply = accessManager->post(request, byteArray);
        //        QNetworkReply *reply = accessManager->post(request, multiPart);
        //        multiPart->setParent(reply); // reply对象将负责删除multiPart对象

        ////        QJsonDocument document;
        ////        document.setObject(json);
        ////        QString str=QString(document.toJson());
        ////        QByteArray byteArray = str.toUtf8();

        ////        qDebug()<<byteArray;
        ////        QNetworkReply* reply = accessManager->post(request, byteArray);
        ////        if(!reply)
        ////            qDebug()<<"reply = nullptr";

        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(reply->error())
        {
            qDebug() << "ERROR!";
            qDebug() << reply->errorString();
        }
        int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"handleTip"<<code;
        QByteArray responseData = reply->readAll();
        vector<NeuronSWC> markPoints;
        if(code==200)
        {
            //解析json
            QJsonParseError json_error;
            QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
            if (json_error.error == QJsonParseError::NoError) {
                if (doucment.isObject()) {
                    const QJsonObject obj = doucment.object();
                    qDebug() << obj;
                    QString objCode;
                    QString objMsg;
                    if (obj.contains("code")) {
                        QJsonValue value = obj.value("code");
                        if (value.isString()) {
                            objCode= value.toString();
                            qDebug() << "code : " << objCode;
                        }
                    }
                    if (obj.contains("msg")) {
                        QJsonValue value = obj.value("msg");
                        if (value.isString()) {
                            QString objMsg = value.toString();
                            qDebug() << "msg : " << objMsg;
                        }
                    }
                    if (obj.contains("data")&&objCode=="200") {
                        QJsonValue value = obj.value("data");
                        if (value.isArray()) {  // Version 的 value 是数组
                            QJsonArray array = value.toArray();
                            int nSize = array.size();
                            for (int i = 0; i < nSize; ++i) {
                                QJsonValue mapValue = array.at(i);
                                if (mapValue.isObject()) {
                                    QJsonObject info = mapValue.toObject();
                                    float x,y,z;
                                    int y_pred;
                                    if (info.contains("coors")) {
                                        QJsonValue listValue = info.value("coors");
                                        if (listValue.isArray()) {
                                            QJsonArray listArray = listValue.toArray();
                                            QJsonValue xValue = listArray.at(0);
                                            QJsonValue yValue = listArray.at(1);
                                            QJsonValue zValue = listArray.at(2);
                                            x=xValue.toInt();
                                            y=yValue.toInt();
                                            z=zValue.toInt();
                                        }
                                    }
                                    if (info.contains("y_pred")) {
                                        QJsonValue predValue = info.value("y_pred");
                                        y_pred = predValue.toInt();
                                        qDebug()<<i<<": "<<y_pred;
                                    }
                                    if(y_pred==1){
                                        NeuronSWC s;
                                        s.x=x;
                                        s.y=y;
                                        s.z=z;
                                        s.type=10;
                                        markPoints.push_back(s);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            std::cerr<<"handle tip error!";
        }
        for(int i=0;i<markPoints.size();i++){
            QStringList result;
            result.push_back(QString("%1 server").arg(0));
            result.push_back(QString("%1 %2 %3 %4").arg(markPoints[i].type).arg(markPoints[i].x).arg(markPoints[i].y).arg(markPoints[i].z));
            QString msg=QString("/WARN_TipUndone:"+result.join(","));
            myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_TipUndone:").size()));
            qDebug()<<"Server finish /WARN_TipUndone";
            emit myServer->clientSendMsgs({msg});
        }

        if(markPoints.size()!=0)
            myServer->imediateSave();

        //清理资源
        reply->deleteLater();     
    }
}

void CollDetection::handleCrossing(vector<vector<NeuronSWC>>& crossingPoints, map<string, vector<string>> &parentsDict, map<string, vector<string>> &offspringsDict){
    for(int i=0;i<crossingPoints.size();i++){
        for(auto it=crossingPoints[i].begin();it!=crossingPoints[i].end();it++){
            QStringList result;
            result.push_back(QString("%1 server").arg(0));
            result.push_back(QString("%1 %2 %3 %4").arg(it->type).arg(it->x).arg(it->y).arg(it->z));
            QString msg=QString("/WARN_Crossing:"+result.join(","));
            myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_Crossing:").size()));
            qDebug()<<"Server finish /WARN_Crossing";
            emit myServer->clientSendMsgs({msg});
        }
    }

    if(crossingPoints.size()!=0){
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        QString swcFileName=myServer->getAnoName()+".ano.eswc";
        QString fileSaveName=myServer->swcpath.left(myServer->swcpath.size()-QString(".ano.eswc").size())+"_sorted.ano.eswc";
        sortSWC(myServer->swcpath,fileSaveName,0);

        // 创建一个QFile对象，用于读取要上传的文件
        QFile *file = new QFile(fileSaveName);
        file->open(QIODevice::Text);
        file->setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
        setSWCRadius(fileSaveName,1);

        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"swcFile\"; filename=\""+ swcFileName + "\"")); // file为后端定义的key，filename即为excel文件名
        filePart.setBodyDevice(file);
        file->setParent(multiPart); // 文件将由multiPart对象进行管理

        multiPart->append(filePart);
        QNetworkRequest fileRequest(QUrl(HostAddress+"/detect/file/for-crossing"));
        // 发送HTTP POST请求
        QNetworkReply* fileReply = accessManager->post(fileRequest, multiPart);
        multiPart->setParent(fileReply); // reply对象将负责删除multiPart对象
        QEventLoop tmpeventLoop;
        connect(fileReply, &QNetworkReply::finished, &tmpeventLoop, &QEventLoop::quit);
        tmpeventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(fileReply->error())
        {
            qDebug() << "SENDFILEERROR!";
            qDebug() << fileReply->errorString();
        }
        int fileResCode=fileReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"sendFile"<<fileResCode;
        //清理资源
        QFile::remove(fileSaveName);
        file->close();
        file->deleteLater();


        QJsonObject json;
        QString obj=myServer->getImage();
        json.insert("obj",obj);
        json.insert("res", myServer->RES);
        QJsonArray infos;
        for(int i=0; i<crossingPoints.size();i++){
            QJsonArray info;
            for(auto it=crossingPoints[i].begin();it!=crossingPoints[i].end();it++){
                QString gridKeyQ = QString::number(it->x) + "_" + QString::number(it->y) + "_" + QString::number(it->z);
                string gridKey = gridKeyQ.toStdString();
                QJsonObject point;
                point.insert("x", it->x);
                point.insert("y", it->y);
                point.insert("z", it->z);
                QJsonArray parentsCoors;
                QJsonArray offspringsCoors;
                for(int j=0;j<parentsDict[gridKey].size();j++){
                    QJsonObject everyParent;
                    XYZ coor;
                    stringToXYZ(parentsDict[gridKey][j],coor.x,coor.y,coor.z);
                    everyParent.insert("x", coor.x);
                    everyParent.insert("y", coor.y);
                    everyParent.insert("z", coor.z);
                    parentsCoors.append(everyParent);
                }
                for(int j=0;j<offspringsDict[gridKey].size();j++){
                    QJsonObject everyOffSpring;
                    XYZ coor;
                    stringToXYZ(offspringsDict[gridKey][j],coor.x,coor.y,coor.z);
                    everyOffSpring.insert("x", coor.x);
                    everyOffSpring.insert("y", coor.y);
                    everyOffSpring.insert("z", coor.z);
                    offspringsCoors.append(everyOffSpring);
                }
                point.insert("parentsCoors",parentsCoors);
                point.insert("offspringsCoors",offspringsCoors);
                info.append(point);
            }
            infos.append(info);
        }
        json.insert("infos",infos);

        //        QFile *file = new QFile(swcpath);
        //        file->open(QIODevice::ReadOnly | QIODevice::Text);
        //        QString content = file->readAll();
        //        QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8());
        //        QJsonObject fileObject = jsonDocument.object();
        //        json.insert("swcFile",fileObject);

        // 创建一个QHttpMultiPart对象，用于构建multipart/form-data请求
        //        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // 创建一个QFile对象，用于读取要上传的文件
        //        QFile *file = new QFile(swcpath);
        //        file->open(QIODevice::ReadOnly | QIODevice::Text);


        // 将文件添加到multipart请求中
        //        QHttpPart filePart;
        //        QString swcFileName=AnoName+".ano.eswc";
        //        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"swcFile\"; filename=\"%1\"").arg(swcFileName)));
        //        filePart.setBodyDevice(file);
        //        file->setParent(multiPart); // 文件将由multiPart对象进行管理

        //        multiPart->append(filePart);
        //        QHttpPart jsonPart;
        //        jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"info\""));
        //        jsonPart.setBody(QJsonDocument(json).toJson());

        //        multiPart->append(jsonPart);

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray=str.toUtf8();

        // 创建一个QNetworkRequest对象，设置URL和请求方法
        QNetworkRequest request(QUrl(HostAddress+"/detect/crossing"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        //        request.setRawHeader("Content-Type", "multipart/form-data; boundary=" + multiPart->boundary());

        // 发送HTTP POST请求
        QNetworkReply* reply = accessManager->post(request, byteArray);
        //        QNetworkReply *reply = accessManager->post(request, multiPart);
        //        multiPart->setParent(reply); // reply对象将负责删除multiPart对象

        ////        QJsonDocument document;
        ////        document.setObject(json);
        ////        QString str=QString(document.toJson());
        ////        QByteArray byteArray = str.toUtf8();

        ////        qDebug()<<byteArray;
        ////        QNetworkReply* reply = accessManager->post(request, byteArray);
        ////        if(!reply)
        ////            qDebug()<<"reply = nullptr";

        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

        if(reply->error())
        {
            qDebug() << "ERROR!";
            qDebug() << reply->errorString();
        }
        int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"handleCrossing"<<code;
        QByteArray responseData = reply->readAll();
        vector<NeuronSWC> markPoints;
        if(code==200)
        {
            //解析json
            QJsonParseError json_error;
            QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
            if (json_error.error == QJsonParseError::NoError) {
                if (doucment.isObject()) {
                    const QJsonObject obj = doucment.object();
                    qDebug() << obj;
                    QString objCode;
                    QString objMsg;
                    if (obj.contains("code")) {
                        QJsonValue value = obj.value("code");
                        if (value.isString()) {
                            objCode= value.toString();
                            qDebug() << "code : " << objCode;
                        }
                    }
                    if (obj.contains("msg")) {
                        QJsonValue value = obj.value("msg");
                        if (value.isString()) {
                            QString objMsg = value.toString();
                            qDebug() << "msg : " << objMsg;
                        }
                    }
                    if (obj.contains("data")&&objCode=="200") {
                        QJsonValue value = obj.value("data");
                        if (value.isArray()) {  // Version 的 value 是数组
                            QJsonArray array = value.toArray();
                            int nSize = array.size();
                            for (int i = 0; i < nSize; ++i) {
                                QJsonValue mapValue = array.at(i);
                                if (mapValue.isObject()) {
                                    QJsonObject info = mapValue.toObject();
                                    float x,y,z;
                                    int y_pred;
                                    if (info.contains("coors")) {
                                        QJsonValue listValue = info.value("coors");
                                        if (listValue.isArray()) {
                                            QJsonArray listArray = listValue.toArray();
                                            QJsonValue xValue = listArray.at(0);
                                            QJsonValue yValue = listArray.at(1);
                                            QJsonValue zValue = listArray.at(2);
                                            x=xValue.toInt();
                                            y=yValue.toInt();
                                            z=zValue.toInt();
                                        }
                                    }
                                    if (info.contains("y_pred")) {
                                        QJsonValue predValue = obj.value("y_pred");
                                        if (predValue.isDouble()) {
                                            y_pred = predValue.toInt();
                                        }
                                    }
                                    if(y_pred==0){
                                        NeuronSWC s;
                                        s.x=x;
                                        s.y=y;
                                        s.z=z;
                                        s.type=12;
                                        markPoints.push_back(s);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            std::cerr<<"handle tip error!";
        }
        for(int i=0;i<markPoints.size();i++){
            QStringList result;
            result.push_back(QString("%1 server").arg(0));
            result.push_back(QString("%1 %2 %3 %4").arg(markPoints[i].type).arg(markPoints[i].x).arg(markPoints[i].y).arg(markPoints[i].z));
            QString msg=QString("/WARN_CrossingError:"+result.join(","));
            myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_CrossingError:").size()));
            qDebug()<<"Server finish /WARN_CrossingError";
            emit myServer->clientSendMsgs({msg});
        }

        if(markPoints.size()!=0)
            myServer->imediateSave();

        //清理资源
        reply->deleteLater();
        file->close();
        file->deleteLater();
    }
}

void CollDetection::sortSWC(QString fileOpenName, QString fileSaveName, double thres, V3DLONG rootid){
    QList<NeuronSWC> neuron, result;
    if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC") || fileOpenName.endsWith(".eswc") || fileOpenName.endsWith(".ESWC"))
        neuron = readSWC_file(fileOpenName).listNeuron;
    if (!SortSWC(neuron, result , rootid, thres))
    {
        cout<<"Error in sorting swc"<<endl;
    }
    if (!export_list2file(result, fileSaveName, fileOpenName))
    {
        cout<<"Error in writing swc to file"<<endl;
    }
}

void CollDetection::setSWCRadius(QString filePath, int r){
    if (filePath.endsWith(".swc") || filePath.endsWith(".SWC") || filePath.endsWith(".eswc") || filePath.endsWith(".ESWC"))
    {
        QFile qf(filePath);
        QString arryRead;
        if(!qf.open(QIODevice::ReadOnly|QIODevice::Text)){
            return;
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
            qDebug()<<"swcfile cannot be opened!";
            return;
        }
        QTextStream streamWrite(&qf);
        for(int i=0;i<arryListWrite.size()-1;i++){      //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
            if(arryListWrite.at(i).contains("#")){
                streamWrite<<arryListWrite.at(i)<<"\n";
            }else{
                QString contentWrite= arryListWrite.at(i);
                QStringList swcInfo=contentWrite.split(' ',Qt::SkipEmptyParts);
                swcInfo[5]=QString::number(r);
                contentWrite=swcInfo.join(' ');
                streamWrite<<contentWrite<<"\n";
            }
        }
        qf.close();
    }
}
