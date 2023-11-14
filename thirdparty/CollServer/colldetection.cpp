#include "colldetection.h"
#include "coll_server.h"
#include "sort_swc.h"

XYZ CollDetection::maxRes;
XYZ CollDetection::subMaxRes;

CollDetection::CollDetection(CollServer* curServer, QObject* parent):myServer(static_cast<CollServer*>(parent)){
    myServer=curServer;
    accessManager=new QNetworkAccessManager(this);
    timerForFilterTip=new QTimer(this);
    SuperUserHostAddress="http://114.117.165.134:26000/SuperUser";
    BrainTellHostAddress="http://114.117.165.134:26000/dynamic";
}

XYZ CollDetection::getSomaCoordinate(QString apoPath){
    myServer->isSomaExists=false;
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
                myServer->isSomaExists=true;
                break;
            }
        }
    }
    return coordinate;
}

void CollDetection::detectTips(){
    myServer->mutex.lock();
    myServer->mutexForDetectMissing.lock();
    map<string, set<size_t>> allPoint2SegIdMap = getWholeGrid2SegIDMap(myServer->segments);
    getSegmentsForMissingDetect(myServer->last3MinSegments, myServer->segmentsForMissingDetect, myServer->segments);

    tipPoints=tipDetection(myServer->segmentsForMissingDetect, false, allPoint2SegIdMap, 30);
//    QString apoFileNameTip="/home/BrainTellServer/image/tmpApoFile/tip/"+myServer->getAnoName()+"_tip.apo";
//    getApoForCrop(apoFileNameTip, tipPoints);
//    myServer->imediateSave();
    myServer->last3MinSegments.seg.clear();
    myServer->segmentsForMissingDetect.seg.clear();
    myServer->mutexForDetectMissing.unlock();
    myServer->mutex.unlock();

    timerForFilterTip->start(1*60*1000);
//    tipPoints=tipDetection(myServer->segments, true, 20);
//    myServer->imediateSave();

//    tipPoints=tipDetection(myServer->segments, false, 20);
//    QString apoFileNameUndone="/home/BrainTellServer/image/tmpApoFile/undone/"+myServer->getAnoName()+"_undone.apo";
//    getApoForCrop(apoFileNameUndone, tipPoints);
//    myServer->imediateSave();
//    handleTip(tipPoints);
}

void CollDetection::detectCrossings(){
    map<string, vector<string>> parentsDict;
    map<string, vector<string>> offspringsDict;

    vector<vector<NeuronSWC>> crossingPoints=crossingDetection(myServer->segments, parentsDict, offspringsDict);
    handleCrossing(crossingPoints, parentsDict, offspringsDict);
}

void CollDetection::detectOthers(){
    myServer->mutex.lock();
    myServer->mutexForDetectOthers.lock();
    getSegmentsForOthersDetect(myServer->last1MinSegments, myServer->segmentsForOthersDetect, myServer->segments);

    int count=0;
    vector<NeuronSWC> outputSpecialPoints = specStructsDetection(myServer->segmentsForOthersDetect);
    myServer->last1MinSegments.seg.clear();
    myServer->segmentsForOthersDetect.seg.clear();
    myServer->mutexForDetectOthers.unlock();
    myServer->mutex.unlock();

    vector<NeuronSWC> bifurPoints;
    vector<NeuronSWC> mulfurPoints;

    for(int i=0;i<outputSpecialPoints.size();i++){
        if(outputSpecialPoints[i].type == 6)
            bifurPoints.push_back(outputSpecialPoints[i]);
        else if(outputSpecialPoints[i].type == 8)
            mulfurPoints.push_back(outputSpecialPoints[i]);
    }

    handleMulFurcation(mulfurPoints, count);
    handleNearBifurcation(bifurPoints, count);

}

void CollDetection::detectLoops(){
    int count=0;
    myServer->mutex.lock();
    vector<NeuronSWC> outputSpecialPoints = loopDetection(myServer->segments);
    myServer->mutex.unlock();

    handleLoop(outputSpecialPoints, count);
}

vector<NeuronSWC> CollDetection::specStructsDetection(V_NeuronSWC_list& inputSegList, double dist_thresh){
    vector<NeuronSWC> outputSpecialPoints;
    if(inputSegList.seg.size()==0)
        return outputSpecialPoints;

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
//                    qDebug()<<*it1;
//                    qDebug()<<getSegLength(inputSegList.seg[*it1]);
                    if(getSegLength(inputSegList.seg[*it1])>40)
                        count1++;
                }

                for(auto it2=n2Segs.begin();it2!=n2Segs.end();it2++)
                {
//                    qDebug()<<*it2;
//                    qDebug()<<getSegLength(inputSegList.seg[*it2]);
                    if(getSegLength(inputSegList.seg[*it2])>40)
                        count2++;
                }
//                qDebug()<<"n2Segs end";
                if(!(count1>=2&&count2>=2)){
                    continue;
                }
                if(myServer->isSomaExists){
                    if(distance(n1.x,myServer->somaCoordinate.x,n1.y,myServer->somaCoordinate.y,n1.z,myServer->somaCoordinate.z)>soma_radius
                        &&distance(n2.x,myServer->somaCoordinate.x,n2.y,myServer->somaCoordinate.y,n2.z,myServer->somaCoordinate.z)>soma_radius){
                        double dist=distance(n1.x,n2.x,n1.y,n2.y,n1.z,n2.z);
                        if(distance((n1.x+n2.x)/2,myServer->somaCoordinate.x,(n1.y+n2.y)/2,myServer->somaCoordinate.y,(n1.z+n2.z)/2,myServer->somaCoordinate.z)>1e-7&&dist<dist_thresh){
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

    set<size_t> overlapSegIds;
    //检测overlap线段
    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        for(size_t j=i+1; j<inputSegList.seg.size(); j++){
            int result = isOverlapOfTwoSegs(inputSegList.seg[i], inputSegList.seg[j]);
            if(result == 1)
                overlapSegIds.insert(i);
            if(result == 2)
                overlapSegIds.insert(j);
        }
    }
    for(auto it=overlapSegIds.begin(); it!=overlapSegIds.end(); it++){
        auto seg_it = findseg(myServer->segments.seg.begin(), myServer->segments.seg.end(), inputSegList.seg[*it]);
        if(seg_it != myServer->segments.seg.end()){
            seg_it->to_be_deleted = true;
        }
    }

    std::vector<V_NeuronSWC>::iterator iter = myServer->segments.seg.begin();
    while (iter != myServer->segments.seg.end())
        if (iter->to_be_deleted){
            QStringList result;
            result.push_back(QString("%1 server %2 %3 %4").arg(0).arg(123).arg(123).arg(123));
            result+=V_NeuronSWCToSendMSG(*iter);
            QString msg=QString("/delline_norm:"+result.join(","));
            emit myServer->clientSendMsgs({msg});

            iter = myServer->segments.seg.erase(iter);
        }
        else
            ++iter;

    for(auto it=pset.begin(); it!=pset.end(); it++){
        qDebug()<<*it;
        NeuronSWC n;
        stringToXYZ(points[*it],n.x,n.y,n.z);
        n.type=6;
        outputSpecialPoints.push_back(n);
    }

    return outputSpecialPoints;

}

vector<NeuronSWC> CollDetection::loopDetection(V_NeuronSWC_list& inputSegList){
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
    size_t start=0;
    for(size_t i=0; i<newpoints.size(); ++i){
//        qDebug()<<QString::fromStdString(newpoints[i])<<" "<<parentMap[newpoints[i]].size();
        /*if(newLinksIndexVec[i].size()>=2&&counts[i]>=3&&newLinksIndexVec[i].size()!=counts[i])*/
        if(parentMap[newpoints[i]].size()>=2){
//            size_t interval=i-start;
//            //interval/12
//            int nums=interval/8;
//            for(int j=0;j<nums;j++){
//                specPoints.insert(newpoints[start+(j+1)*8]);
//            }

            specPoints.insert(newpoints[i]);
//            start=i+1;
//            qDebug()<<"loop exists";
        }

    }

//    if(start<newpoints.size()){
//        size_t interval=newpoints.size()-1-start;
//        int nums=interval/8;
//        for(int j=0;j<nums;j++){
//            specPoints.insert(newpoints[start+(j+1)*8]);
//        }
//    }

    //检测2条边构成的环
    for(size_t i=0; i<inputSegList.seg.size(); ++i){
        V_NeuronSWC seg = inputSegList.seg[i];
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
        set<size_t> segSet1=wholeGrid2segIDmap[gridKey1];
        set<size_t> segSet2=wholeGrid2segIDmap[gridKey2];
        set<size_t> intersectionSet;
        set_intersection(segSet1.begin(),segSet1.end(),segSet2.begin(),segSet2.end(),inserter( intersectionSet , intersectionSet.begin()));
        vector<size_t> maybeOverlapSegIdVec;
        set<size_t> overlapSegIdSet;

        if(intersectionSet.size()>=2){
            for(auto it=intersectionSet.begin(); it!=intersectionSet.end(); it++){
                maybeOverlapSegIdVec.push_back(*it);
            }
            for(int m=0; m<maybeOverlapSegIdVec.size(); m++){
                for(int n=m+1; n<maybeOverlapSegIdVec.size(); n++){
                    int result = isOverlapOfTwoSegs(inputSegList.seg[maybeOverlapSegIdVec[m]], inputSegList.seg[maybeOverlapSegIdVec[n]]);
                    if(result == 1)
                        overlapSegIdSet.insert(maybeOverlapSegIdVec[m]);
                    if(result == 2)
                        overlapSegIdSet.insert(maybeOverlapSegIdVec[n]);
                }
            }

            if(maybeOverlapSegIdVec.size() - overlapSegIdSet.size() != 1){
                qDebug()<<"exists two-edge loop";
                qDebug()<<"gridKey1:"<<QString::fromStdString(gridKey1);
                qDebug()<<"gridKey2:"<<QString::fromStdString(gridKey2);
                specPoints.insert(gridKey1);
                specPoints.insert(gridKey2);
            }

            for(auto it=overlapSegIdSet.begin(); it!=overlapSegIdSet.end(); it++){
//                qDebug()<<"to be deleted";
//                qDebug()<<*it;
                inputSegList.seg[*it].to_be_deleted = true;
            }
        }
    }

    std::vector<V_NeuronSWC>::iterator iter = inputSegList.seg.begin();
    while (iter != inputSegList.seg.end())
        if (iter->to_be_deleted){
            QStringList result;
            result.push_back(QString("%1 server %2 %3 %4").arg(0).arg(123).arg(123).arg(123));
            result+=V_NeuronSWCToSendMSG(*iter);
            QString msg=QString("/delline_norm:"+result.join(","));
            emit myServer->clientSendMsgs({msg});

            iter = inputSegList.seg.erase(iter);
        }
        else
            ++iter;

    for(auto it=specPoints.begin(); it!=specPoints.end(); it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        s.type=0;
        outputSpecialPoints.push_back(s);
    }

    return outputSpecialPoints;
}

vector<NeuronSWC> CollDetection::tipDetection(V_NeuronSWC_list &inputSegList, bool flag, map<string, set<size_t>> allPoint2SegIdMap, double dist_thresh){
    vector<NeuronSWC> outputSpecialPoints;
    if(inputSegList.seg.size()==0)
        return outputSpecialPoints;

    XYZ res;
    // 定义正则表达式
    QRegularExpression regex("RES\\((\\d+)x(\\d+)x(\\d+)\\)");

    // 进行匹配
    QRegularExpressionMatch match = regex.match(myServer->RES);

    // 检查匹配是否成功
    if (match.hasMatch()) {
        // 提取宽度、高度和深度
        QString yStr = match.captured(1);
        QString xStr = match.captured(2);
        QString zStr = match.captured(3);

        res.x=xStr.toInt();
        res.y=yStr.toInt();
        res.z=zStr.toInt();
    } else {
        qDebug() << "No match found";
        return outputSpecialPoints;
    }

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
        if(wholeGrid2segIDmap[gridKey1].size()==1 && allPoint2SegIdMap[gridKey1].size()==1 && wholeGrid2segIDmap[gridKey2].size()>1)
        {
            if(myServer->isSomaExists&&sqrt((xLabel1-myServer->somaCoordinate.x)*(xLabel1-myServer->somaCoordinate.x)+
                (yLabel1-myServer->somaCoordinate.y)*(yLabel1-myServer->somaCoordinate.y)+(zLabel1-myServer->somaCoordinate.z)*(zLabel1-myServer->somaCoordinate.z))>50)
                tips.insert(gridKey1);
            else if(!myServer->isSomaExists)
                tips.insert(gridKey1);
        }
        if(wholeGrid2segIDmap[gridKey2].size()==1 && allPoint2SegIdMap[gridKey2].size()==1 && wholeGrid2segIDmap[gridKey1].size()>1)
        {
            if(myServer->isSomaExists&&sqrt((xLabel2-myServer->somaCoordinate.x)*(xLabel2-myServer->somaCoordinate.x)+
                (yLabel2-myServer->somaCoordinate.y)*(yLabel2-myServer->somaCoordinate.y)+(zLabel2-myServer->somaCoordinate.z)*(zLabel2-myServer->somaCoordinate.z))>50)
                tips.insert(gridKey2);
            else if(!myServer->isSomaExists)
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
            if(s.x>65&&s.x+65<res.x&&s.y>65&&s.y+65<res.y&&s.z>65&&s.z+65<res.z)
                outputSpecialPoints.push_back(s);
        }

    }

    if(!flag)
        return outputSpecialPoints;

    vector<V_NeuronSWC> delSegVec;
    for(auto it=outputSpecialPoints.begin(); it!=outputSpecialPoints.end(); it++){
        float xLabel = it->x;
        float yLabel = it->y;
        float zLabel = it->z;
        QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
        string gridKey = gridKeyQ.toStdString();

        size_t segId=*wholeGrid2segIDmap[gridKey].begin();

        V_NeuronSWC& seg = inputSegList.seg[segId];

        float xLabel0 = seg.row[0].x;
        float yLabel0 = seg.row[0].y;
        float zLabel0 = seg.row[0].z;
        QString gridKeyQ0 = QString::number(xLabel0) + "_" + QString::number(yLabel0) + "_" + QString::number(zLabel0);
        string gridKey0 = gridKeyQ0.toStdString();

        bool isReverse=false;
        if(wholeGrid2segIDmap[gridKey0].size()!=1)
        {
            isReverse=true;
        }

        int size=seg.row.size();
        vector<int> indexs(size);
        for(int m=0;m<size;m++)
            indexs[m]=m;
        if(isReverse)
            reverse(indexs.begin(),indexs.end());

        if(size<=7){
            auto segIt=findseg(inputSegList.seg.begin(),inputSegList.seg.end(),seg);
            if(segIt!=inputSegList.seg.end())
            {
                delSegVec.push_back(seg);
            }
            continue;
        }

        qDebug()<<"before: ";
        seg.printInfo();

        int count=1;
        while(count<=6){
            auto tmpIt = seg.row.end();
            if(!isReverse){
                tmpIt=seg.row.begin();
            }
            else{
                tmpIt=seg.row.end()-1;
            }
            seg.row.erase(tmpIt);
            count++;
        }

        int nodeNo = 1;
        for (vector<V_NeuronSWC_unit>::iterator it_unit = seg.row.begin();
             it_unit != seg.row.end(); it_unit++)
        {
            it_unit->data[0] = nodeNo;
            it_unit->data[6] = nodeNo + 1;
            ++nodeNo;
        }
        (seg.row.end()-1)->data[6]=-1;
        qDebug()<<"after: ";
        seg.printInfo();
    }

    for(auto delIt=delSegVec.begin(); delIt!=delSegVec.end(); delIt++){
        auto segIt=findseg(inputSegList.seg.begin(),inputSegList.seg.end(),*delIt);
        if(segIt!=inputSegList.seg.end())
        {
            inputSegList.seg.erase(segIt);
        }
    }

    return outputSpecialPoints;
}

vector<vector<NeuronSWC>> CollDetection::crossingDetection(V_NeuronSWC_list& inputSegList, map<string, vector<string>> &parentsDict, map<string, vector<string>> &offspringsDict){
    vector<vector<NeuronSWC>> outputSpecialPoints;
    XYZ res;
    // 定义正则表达式
    QRegularExpression regex("RES\\((\\d+)x(\\d+)x(\\d+)\\)");

    // 进行匹配
    QRegularExpressionMatch match = regex.match(myServer->RES);

    // 检查匹配是否成功
    if (match.hasMatch()) {
        // 提取宽度、高度和深度
        QString yStr = match.captured(1);
        QString xStr = match.captured(2);
        QString zStr = match.captured(3);

        res.x=xStr.toInt();
        res.y=yStr.toInt();
        res.z=zStr.toInt();
    } else {
        qDebug() << "No match found";
        return outputSpecialPoints;
    }

    map<string, set<string>> parentMap;

    set<string> allPoints;
    map<string, set<string>> childMap;
    map<string, set<size_t> > wholeGrid2segIDmap;

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
            wholeGrid2segIDmap[gridKey].insert(size_t(i));

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

//    for(auto it:parentMap){
//        qDebug()<<QString::fromStdString(it.first)<<"======"<<it.second.size();
//        //        for(auto it2=it.second.begin();it2!=it.second.end();it2++){
//        //            qDebug()<<QString::fromStdString(*it2);
//        //        }
//    }
//    qDebug()<<"!!!!!!!!!!!!!!!!!!!";
//    for(auto it:childMap){
//        qDebug()<<QString::fromStdString(it.first)<<"======"<<it.second.size();
////        for(auto it2=it.second.begin();it2!=it.second.end();it2++){
////            qDebug()<<QString::fromStdString(*it2);
////        }
//    }

    //get_soma_nearby_points
    double ignore_radius_from_soma = 50;
    set<string> remain_pts;
    qDebug()<<"isSomaExists: "<<myServer->isSomaExists;
    for(auto it=allPoints.begin();it!=allPoints.end();it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        if(!myServer->isSomaExists)
            remain_pts.insert(*it);
        else if(myServer->isSomaExists&&distance(s.x,myServer->somaCoordinate.x,s.y,myServer->somaCoordinate.y,s.z,myServer->somaCoordinate.z)>ignore_radius_from_soma)
            remain_pts.insert(*it);
    }

//    qDebug()<<"00000000000000000000";
    //get_linkages_with_thresh
    int offspring_thresh=10;
    for(auto it=allPoints.begin();it!=allPoints.end();it++){
        string gridKey=*it;
        int os_id=0;
        set<string> cur_set;
        vector<string> cur_vector;
        while(os_id<offspring_thresh){
            if(parentMap.count(gridKey)==0||parentMap[gridKey].size()==0){
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
//            qDebug()<<"??????";
            if(childMap.count(gridKey)==0||childMap[gridKey].size()==0){
                break;
            }
            if(childMap[gridKey].size()==1){
                bool result=cur_set.insert(*childMap[gridKey].begin()).second;
                if(!result)
                    break;
//                qDebug()<<QString::fromStdString(*childMap[gridKey].begin());
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
//        qDebug()<<"3333333333333";
        offspringsDict[*it]=cur_vector;
    }

//    qDebug()<<"parentsDict: ";
//    for(auto it:parentsDict){
//        qDebug()<<QString::fromStdString(it.first)<<"======"<<it.second.size();
//        for(auto it2=it.second.begin();it2!=it.second.end();it2++){
//            qDebug()<<QString::fromStdString(*it2);
//        }
//    }
//    qDebug()<<"!!!!!!!!!!!!!!!";
//    qDebug()<<"offspringsDict: ";
//    for(auto it:offspringsDict){
//        qDebug()<<QString::fromStdString(it.first)<<"======"<<it.second.size();
//        for(auto it2=it.second.begin();it2!=it.second.end();it2++){
//            qDebug()<<QString::fromStdString(*it2);
//        }
//    }
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

//    qDebug()<<"1111111111111111";

    //calc_pairwise_dist
    set<string> d1_coord_set;
    set<set<string>> dmin_coord_listAll ;
    set<size_t> curPairSegs;
    set<set<size_t>> visitedPairSegs;
    for(auto it=remain_pts.begin();it!=remain_pts.end();it++){
        NeuronSWC s;
        stringToXYZ(*it,s.x,s.y,s.z);
        set<string> cur_set(parentsDict[*it].begin(),parentsDict[*it].end());
//        for(auto tmpIt=cur_set.begin();tmpIt!=cur_set.end();tmpIt++)
//            qDebug()<<QString::fromStdString(*tmpIt);
//        qDebug()<<"===================";
        if(cur_set.size()<5)
            continue;
        cur_set.insert(*it);
        if(offspringsDict.count(*it)!=0){
            if(offspringsDict[*it].size()<5)
                continue;
            set<string> offSet(offspringsDict[*it].begin(),offspringsDict[*it].end());
            set_union(cur_set.begin(), cur_set.end(), offSet.begin(), offSet.end(), inserter(cur_set,cur_set.begin()));
//            for(auto tmpIt=cur_set.begin();tmpIt!=cur_set.end();tmpIt++){
//                qDebug()<<QString::fromStdString(*tmpIt);
//            }
//            qDebug()<<"===================";

//            qDebug()<<"s:"<<s.x<<" "<<s.y<<" "<<s.z;
            set<string> pts;
            set_difference(allPoints.begin(),allPoints.end(),cur_set.begin(),cur_set.end(), inserter(pts, pts.begin()));
//            for(auto tmpIt=pts.begin();tmpIt!=pts.end();tmpIt++){
//                qDebug()<<QString::fromStdString(*tmpIt);
//            }
            set<string> nearPoints;
            for(auto coorIt=pts.begin();coorIt!=pts.end();coorIt++){
                NeuronSWC tmp;
                stringToXYZ(*coorIt,tmp.x,tmp.y,tmp.z);
                if(fabs(s.x-tmp.x)<5&&fabs(s.y-tmp.y)<5&&fabs(s.z-tmp.z)<5){
                    nearPoints.insert(*coorIt);
                }
            }
//            if(nearPoints.size()>0){
//                qDebug()<<"s:"<<s.x<<" "<<s.y<<" "<<s.z;
//                for(auto tmpIt=nearPoints.begin();tmpIt!=nearPoints.end();tmpIt++)
//                    qDebug()<<QString::fromStdString(*tmpIt);
//                qDebug()<<"parentsDict[*it]: ";
//                for(auto pr0=parentsDict[*it].begin();pr0!=parentsDict[*it].end();pr0++){
//                    qDebug()<<QString::fromStdString(*pr0);
//                }
//                qDebug()<<"--------------";
//            }


            double cur_dmin=1.5;
            string targetCoor="";
            for(auto coorIt=nearPoints.begin();coorIt!=nearPoints.end();coorIt++){
                set<string> coor_cur_set(parentsDict[*coorIt].begin(),parentsDict[*coorIt].end());
                if(coor_cur_set.size()<5)
                    continue;
                coor_cur_set.insert(*coorIt);
                if(offspringsDict.count(*coorIt)!=0){
                    if(offspringsDict[*coorIt].size()<5)
                        continue;
                    set<string> coor_offSet(offspringsDict[*coorIt].begin(),offspringsDict[*coorIt].end());
                    set_union(coor_cur_set.begin(), coor_cur_set.end(), coor_offSet.begin(), coor_offSet.end(), inserter(coor_cur_set,coor_cur_set.begin()));
                }else{
                    continue;
                }
                NeuronSWC tmp;
                stringToXYZ(*coorIt,tmp.x,tmp.y,tmp.z);
                double dist=distance(s.x,tmp.x,s.y,tmp.y,s.z,tmp.z);
                if(dist<=cur_dmin){
//                    qDebug()<<QString("parentsDict[%1]").arg(QString::fromStdString(*coorIt));
//                    for(auto pr1=parentsDict[*coorIt].begin();pr1!=parentsDict[*coorIt].end();pr1++){
//                        qDebug()<<QString::fromStdString(*pr1);
//                    }
                    set<string> intersectionSet;
                    set_intersection(cur_set.begin(), cur_set.end(), coor_cur_set.begin(), coor_cur_set.end(), inserter(intersectionSet, intersectionSet.begin()));
                    //                    for(auto pr0=parentsDict[*it].begin();pr0!=parentsDict[*it].end();pr0++){
                    //                        for(auto pr1=parentsDict[*coorIt].begin();pr1!=parentsDict[*coorIt].end();pr1++){
                    //                            if(*pr0==*pr1){
                    //                                hasCommonParents=true;
                    //                                break;
                    //                            }
                    //                        }
                    //                        if(hasCommonParents==true)
                    //                            break;
                    //                    }
                    if(intersectionSet.size()==0){
                        cur_dmin=dist;
                        targetCoor=*coorIt;
                    }
                }
            }
            if(targetCoor!=""){
                for(auto tmpIt=wholeGrid2segIDmap[targetCoor].begin();tmpIt!=wholeGrid2segIDmap[targetCoor].end();tmpIt++){
                    curPairSegs.insert(*tmpIt);
                }
                for(auto tmpIt=wholeGrid2segIDmap[*it].begin();tmpIt!=wholeGrid2segIDmap[*it].end();tmpIt++){
                    curPairSegs.insert(*tmpIt);
                }
                bool result=visitedPairSegs.insert(curPairSegs).second;
                if(result){
                    d1_coord_set.insert(targetCoor);
                    d1_coord_set.insert(*it);
                    dmin_coord_listAll.insert(d1_coord_set);
                    d1_coord_set.clear();
                }
                curPairSegs.clear();
            }
//            qDebug()<<"==================";
        }else {
            continue;
        }
    }

    for(auto it=dmin_coord_listAll.begin();it!=dmin_coord_listAll.end();it++)
    {
        vector<NeuronSWC> tmpVec;
        bool flag=true;
        NeuronSWC s;
        s.type=15;
        for(auto coorIt=(*it).begin();coorIt!=(*it).end();coorIt++){
            stringToXYZ(*coorIt,s.x,s.y,s.z);
            if(s.x>65&&s.x+65<res.x&&s.y>65&&s.y+65<res.y&&s.z>65&&s.z+65<res.z)
                tmpVec.push_back(s);
            else{
                flag=false;
                break;
            }
            qDebug()<<QString::fromStdString(*coorIt);
        }
        if(!flag)
            continue;
        qDebug()<<"++++++++++";
        outputSpecialPoints.push_back(tmpVec);
    }
    return outputSpecialPoints;
}

void CollDetection::handleMulFurcation(vector<NeuronSWC>& outputSpecialPoints, int& count){
    for(int i=0;i<outputSpecialPoints.size();i++){
        if(myServer->isSomaExists)
        {
            if(distance(outputSpecialPoints[i].x, myServer->somaCoordinate.x, outputSpecialPoints[i].y, myServer->somaCoordinate.y,
                         outputSpecialPoints[i].z, myServer->somaCoordinate.z) > 12)
            {
                count++;
                QStringList result;
                result.push_back(QString("server"));
                result.push_back(QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z));
                QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
                //            const std::string data=msg.toStdString();
                //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();

                //emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size());
                bool isSucess=myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));

                if(isSucess)
                    emit myServer->clientSendMsgs({msg});
            }
        }

        else{
            count++;
            QStringList result;
            result.push_back(QString("server"));
            result.push_back(QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z));
            QString msg=QString("/WARN_MulBifurcation:"+result.join(","));
            //            const std::string data=msg.toStdString();
            //            const std::string header=QString("DataTypeWithSize:%1 %2\n").arg(0).arg(data.size()).toStdString();
            //                auto sockets=hashmap.values();
            //                emit clientAddMarker(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));
            bool isSucess=myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_MulBifurcation:").size()));

            if(isSucess)
                emit myServer->clientSendMsgs({msg});
        }
    }
    qDebug()<<"Server finish /WARN_MulBifurcation";
}

void CollDetection::handleLoop(vector<NeuronSWC>& outputSpecialPoints, int& count){
    QString tobeSendMsg=QString("/WARN_Loop:server,");
    QStringList result;

    for(int i=0;i<outputSpecialPoints.size();i++){
        QString curMarker=QString("%1 %2 %3 %4").arg(outputSpecialPoints[i].type).arg(outputSpecialPoints[i].x).arg(outputSpecialPoints[i].y).arg(outputSpecialPoints[i].z);

        QString msg=QString("/WARN_Loop:server,"+curMarker);
        bool isSucess=myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_Loop:").size()));
        if(isSucess){
            result.push_back(curMarker);
            count++;
        }
    }

    tobeSendMsg = tobeSendMsg + result.join(",");

    if(count!=0)
        emit myServer->clientSendMsgs({tobeSendMsg});
    qDebug()<<"Server finish /WARN_Loop";
}

void CollDetection::handleNearBifurcation(vector<NeuronSWC>& bifurPoints, int& count){
    count+=bifurPoints.size();
    for(int i=0;i<bifurPoints.size();i++){
        QStringList result;
        result.push_back(QString("server"));
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
        QString fileSaveName=myServer->swcpath.left(myServer->swcpath.size()-QString(".ano.eswc").size())+"_copyed.ano.eswc";

        QFile sourceFile(myServer->swcpath);
        if (sourceFile.exists()) {
            // 如果源文件存在，尝试将其复制到目标文件
            if (sourceFile.copy(fileSaveName)) {
                qDebug() << "文件复制成功！";
            } else {
                qDebug() << "文件复制失败：" << sourceFile.errorString();
            }
        } else {
            qDebug() << "源文件不存在：" << myServer->swcpath;
        }
//        sortSWC(myServer->swcpath,fileSaveName,0);

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
        QNetworkRequest fileRequest(QUrl(SuperUserHostAddress+"/detect/file/for-missing"));
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

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray=str.toUtf8();

        // 创建一个QNetworkRequest对象，设置URL和请求方法
        QNetworkRequest request(QUrl(SuperUserHostAddress+"/detect/missing"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        //        request.setRawHeader("Content-Type", "multipart/form-data; boundary=" + multiPart->boundary());

        // 发送HTTP POST请求
        QNetworkReply* reply = accessManager->post(request, byteArray);

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
                                        if(y_pred == 1)
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

        QString tobeSendMsg=QString("/WARN_TipUndone:server,");
        QStringList result;
        int count = 0;

        for(int i=0;i<markPoints.size();i++){
            QString curMarker=QString("%1 %2 %3 %4").arg(markPoints[i].type).arg(markPoints[i].x).arg(markPoints[i].y).arg(markPoints[i].z);

            QString msg=QString("/WARN_TipUndone:server,"+curMarker);
            bool isSucess=myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_TipUndone:").size()));
            if(isSucess){
                result.push_back(curMarker);
                count++;
            }
        }

        tobeSendMsg = tobeSendMsg + result.join(",");

        if(count!=0)
            emit myServer->clientSendMsgs({tobeSendMsg});

        qDebug()<<"Server finish /WARN_TipUndone";

//        if(markPoints.size()!=0)
//            myServer->imediateSave();

        //清理资源
        reply->deleteLater();     
    }
}

void CollDetection::filterTip(){
    timerForFilterTip->stop();
    vector<NeuronSWC> finalTipPoints;
    map<string, set<size_t>> wholeGrid2SegIDMap = getWholeGrid2SegIDMap(myServer->segments);

    for(int i=0; i<tipPoints.size(); i++){
        float xLabel = tipPoints[i].x;
        float yLabel = tipPoints[i].y;
        float zLabel = tipPoints[i].z;
        QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
        string gridKey = gridKeyQ.toStdString();
        if(wholeGrid2SegIDMap.find(gridKey) != wholeGrid2SegIDMap.end() && wholeGrid2SegIDMap[gridKey].size() == 1)
            finalTipPoints.push_back(tipPoints[i]);
    }

    tipPoints.clear();
    handleTip(finalTipPoints);
}

void CollDetection::handleCrossing(vector<vector<NeuronSWC>>& crossingPoints, map<string, vector<string>> &parentsDict, map<string, vector<string>> &offspringsDict){
//    for(int i=0;i<crossingPoints.size();i++){
//        for(auto it=crossingPoints[i].begin();it!=crossingPoints[i].end();it++){
//            QStringList result;
//            result.push_back(QString("%1 server").arg(0));
//            result.push_back(QString("%1 %2 %3 %4").arg(it->type).arg(it->x).arg(it->y).arg(it->z));
//            QString msg=QString("/WARN_Crossing:"+result.join(","));
//            myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_Crossing:").size()));
//            qDebug()<<"Server finish /WARN_Crossing";
//            emit myServer->clientSendMsgs({msg});
//        }
//    }

    if(crossingPoints.size()!=0){
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        QString swcFileName=myServer->getAnoName()+".ano.eswc";
        QString fileSaveName=myServer->swcpath.left(myServer->swcpath.size()-QString(".ano.eswc").size())+"_sorted.ano.eswc";
//        sortSWC(myServer->swcpath,fileSaveName,0);
        // 创建源文件对象
        QFile sourceFile(myServer->swcpath);

        // 打开源文件以进行读取
        if (sourceFile.open(QIODevice::ReadOnly)) {
            // 创建目标文件对象
            QFile destinationFile(fileSaveName);

            // 打开目标文件以进行写入，如果文件不存在则会创建
            if (destinationFile.open(QIODevice::WriteOnly)) {
                // 读取源文件的内容并写入目标文件
                QByteArray data = sourceFile.readAll();
                destinationFile.write(data);

                // 关闭目标文件
                destinationFile.close();
            } else {
                // 处理目标文件无法打开的情况
                qDebug() << "无法打开目标文件：" << destinationFile.errorString();
            }

            // 关闭源文件
            sourceFile.close();
        } else {
            // 处理源文件无法打开的情况
            qDebug() << "无法打开源文件：" << sourceFile.errorString();
        }

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
        QNetworkRequest fileRequest(QUrl(SuperUserHostAddress+"/detect/file/for-crossing"));
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

        QJsonDocument document;
        document.setObject(json);
        QString str=QString(document.toJson());
        QByteArray byteArray=str.toUtf8();

        // 创建一个QNetworkRequest对象，设置URL和请求方法
        QNetworkRequest request(QUrl(SuperUserHostAddress+"/detect/crossing"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
        //        request.setRawHeader("Content-Type", "multipart/form-data; boundary=" + multiPart->boundary());

        // 发送HTTP POST请求
        QNetworkReply* reply = accessManager->post(request, byteArray);

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
                                        QJsonValue predValue = info.value("y_pred");
                                        y_pred = predValue.toInt();
                                        if(y_pred == 0)
                                            qDebug()<<i<<": "<<y_pred;

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
            std::cerr<<"handle crossing error!";
        }

        QString tobeSendMsg=QString("/WARN_CrossingError:server,");
        QStringList result;
        int count = 0;

        for(int i=0;i<markPoints.size();i++){
            QString curMarker=QString("%1 %2 %3 %4").arg(markPoints[i].type).arg(markPoints[i].x).arg(markPoints[i].y).arg(markPoints[i].z);

            QString msg=QString("/WARN_CrossingError:server,"+curMarker);
            bool isSucess=myServer->addmarkers(msg.trimmed().right(msg.size()-QString("/WARN_CrossingError:").size()));
            if(isSucess){
                result.push_back(curMarker);
                count++;
            }
        }

        tobeSendMsg = tobeSendMsg + result.join(",");

        if(count!=0)
            emit myServer->clientSendMsgs({tobeSendMsg});

        qDebug()<<"Server finish /WARN_CrossingError";

//        if(markPoints.size()!=0)
//            myServer->imediateSave();

        //清理资源
        reply->deleteLater();
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

void CollDetection::getImageRES(){
    QJsonObject json;
    QString obj=myServer->getImage();
    QJsonObject userMap;
    userMap.insert("name", "zackzhy");
    userMap.insert("passwd", "123456");
    QJsonObject imageMap;
    imageMap.insert("name", obj);
    imageMap.insert("detail", "");
    json.insert("user", userMap);
    json.insert("Image", imageMap);

    QJsonDocument document;
    document.setObject(json);
    QString str=QString(document.toJson());
    QByteArray byteArray=str.toUtf8();

    // 创建一个QNetworkRequest对象，设置URL和请求方法
    QNetworkRequest request(QUrl(BrainTellHostAddress+"/image/getimagelist"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    // 发送HTTP POST请求
    QNetworkReply* reply = accessManager->post(request, byteArray);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    int code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"getImageRes "<<code;
    QByteArray responseData = reply->readAll();

    QString maxResString;
    QString subMaxResString;
    if(code==200){
        //解析json
        QJsonParseError json_error;
        QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
        if (json_error.error == QJsonParseError::NoError) {
            if (doucment.isArray()) {
                const QJsonArray array = doucment.array();
//                qDebug() << array;
                int nSize = array.size();
                for (int i = 0; i < nSize; ++i) {
                    QJsonValue mapValue = array.at(i);
                    if (mapValue.isObject()) {
                        QJsonObject info = mapValue.toObject();
                        QString name="";
                        QString details;
                        if(info.contains("name")){
                            QJsonValue nameVal=info.value("name");
                            if(nameVal.isString()){
                                name=nameVal.toString();
                            }
                        }

                        if(name!=obj)
                            continue;
                        if(info.contains("detail")){
                            QJsonValue detailsVal=info.value("detail");
                            if(detailsVal.isString()){
                                details=detailsVal.toString();
                                QJsonDocument detailsDoc=QJsonDocument::fromJson(details.toUtf8());
                                if(detailsDoc.isArray()){
                                    QJsonArray detailsArray=detailsDoc.array();
                                    maxResString=detailsArray.at(0).toString();
                                    subMaxResString=detailsArray.at(1).toString();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    QRegularExpression regex("^RES\\((\\d+)x(\\d+)x(\\d+)\\)");
    QRegularExpressionMatchIterator matches = regex.globalMatch(maxResString);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString matchedText = match.captured(0);
        QString matchedY = match.captured(1);
        QString matchedX = match.captured(2);
        QString matchedZ = match.captured(3);
        maxRes.x=matchedX.toFloat();
        maxRes.y=matchedY.toFloat();
        maxRes.z=matchedZ.toFloat();
    }

    matches=regex.globalMatch(subMaxResString);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QString matchedText = match.captured(0);
        QString matchedY = match.captured(1);
        QString matchedX = match.captured(2);
        QString matchedZ = match.captured(3);
        subMaxRes.x=matchedX.toFloat();
        subMaxRes.y=matchedY.toFloat();
        subMaxRes.z=matchedZ.toFloat();
    }

    //清理资源
    reply->deleteLater();
}

void CollDetection::getApoForCrop(QString fileSaveName, vector<NeuronSWC> tipPoints){
    qDebug()<<maxRes.x<<" "<<maxRes.y<<" "<<maxRes.z;
    qDebug()<<subMaxRes.x<<" "<<subMaxRes.y<<" "<<subMaxRes.z;
    QList <CellAPO> markers;
    for(int i=0;i<tipPoints.size();i++){
        tipPoints[i].x/=(maxRes.x/subMaxRes.x);
        tipPoints[i].y/=(maxRes.y/subMaxRes.y);
        tipPoints[i].z/=(maxRes.z/subMaxRes.z);
        CellAPO marker;
        marker.color.r=neuron_type_color[2][0];
        marker.color.g=neuron_type_color[2][1];
        marker.color.b=neuron_type_color[2][2];
        marker.x=tipPoints[i].x;
        marker.y=tipPoints[i].y;
        marker.z=tipPoints[i].z;
        markers.append(marker);
    }

    writeAPO_file(fileSaveName, markers);
}
