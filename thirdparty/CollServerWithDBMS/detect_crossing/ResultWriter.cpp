#include "ResultWriter.h"

ResultWriter::ResultWriter(const std::string &jsonFilePath, const std::string &apoFilePath)
        : m_JsonFilePath(jsonFilePath), m_ApoFilePath(apoFilePath) {

}

void ResultWriter::writeData(const std::vector<CrossingDetect::KeyPointsType> &keyPointsList,
                             const std::vector<CrossingDetect::BranchesType> &selectedBranchesList,
                             std::vector<util::ImageResolutionInfo> resolutionInfoList) {
    std::ofstream jsonOutFile;
    jsonOutFile.open(m_JsonFilePath);
    if (!jsonOutFile.is_open()) {
        throw std::runtime_error("Open file failed. File:"+m_JsonFilePath);
    }

    std::ofstream apoOutFile;
    apoOutFile.open(m_ApoFilePath);
    if (!apoOutFile.is_open()) {
        throw std::runtime_error("Open file failed. File:"+m_ApoFilePath);
    }
    apoOutFile << "##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n";

    int idx = 0;
    for (int i = 0; i < keyPointsList.size(); i++) {
        auto &keyPoints = keyPointsList[i];
        auto &selectedBranches = selectedBranchesList[i];
        auto &resolutionInfo = resolutionInfoList[i];

        json jsonObj;
        for (auto &keyPointPair: keyPoints) {
            if (keyPointPair.second) {
                json parentsCoorsP1;
                json offspringsCoorsP1;

                json parentsCoorsP2;
                json offspringsCoorsP2;

                json onePointPairInfo;
                json onePointInfo1;
                json onePointInfo2;

                auto [p1, branch1Idx] = keyPointPair.first.first;
                auto [p2, branch2Idx] = keyPointPair.first.second;

                auto &p1Branch = selectedBranches[branch1Idx].first;
                auto &p2Branch = selectedBranches[branch2Idx].first;

                auto exportNumber = ConfigManager::getInstance().exportParentAndChildNodeNumber;

                auto exportParentAndChildPoint =
                        [exportNumber, resolutionInfo](const std::vector<util::Node> &pBranch,
                                                       util::Node p,
                                                       json &parentsCoorsP,
                                                       json &offspringsCoorsP
                        ) {
                            for (int i = 0; i < pBranch.size(); ++i) {
                                if (pBranch[i].n == p.n) {
                                    if (i + 1 > exportNumber) {
                                        for (int j = i - exportNumber + 1; j < i; ++j) {
                                            json cord;
                                            auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                           {pBranch[j].x, pBranch[j].y,
                                                                                            pBranch[j].z});
                                            cord["x"] = result.x;
                                            cord["y"] = result.y;
                                            cord["z"] = result.z;
                                            parentsCoorsP.push_back(cord);
                                        }

                                        if (pBranch.size() - i - 1 > exportNumber) {
                                            for (int k = i + 1; k < i + exportNumber + 1; ++k) {
                                                json cord;
                                                auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                               {pBranch[k].x,
                                                                                                pBranch[k].y,
                                                                                                pBranch[k].z});
                                                cord["x"] = result.x;
                                                cord["y"] = result.y;
                                                cord["z"] = result.z;
                                                offspringsCoorsP.push_back(cord);
                                            }
                                        } else {
                                            for (int k = i + 1; k < pBranch.size(); k++) {
                                                json cord;
                                                auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                               {pBranch[k].x,
                                                                                                pBranch[k].y,
                                                                                                pBranch[k].z});
                                                cord["x"] = result.x;
                                                cord["y"] = result.y;
                                                cord["z"] = result.z;
                                                offspringsCoorsP.push_back(cord);
                                            }
                                        }
                                    } else {
                                        for (int j = 0; j < i; j++) {
                                            json cord;
                                            auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                           {pBranch[j].x, pBranch[j].y,
                                                                                            pBranch[j].z});
                                            cord["x"] = result.x;
                                            cord["y"] = result.y;
                                            cord["z"] = result.z;
                                            parentsCoorsP.push_back(cord);
                                        }

                                        if (pBranch.size() - i - 1 > exportNumber) {
                                            for (int k = i + 1; k < i + exportNumber + 1; ++k) {
                                                json cord;
                                                auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                               {pBranch[k].x,
                                                                                                pBranch[k].y,
                                                                                                pBranch[k].z});
                                                cord["x"] = result.x;
                                                cord["y"] = result.y;
                                                cord["z"] = result.z;
                                                offspringsCoorsP.push_back(cord);
                                            }
                                        } else {
                                            for (int k = i + 1; k < pBranch.size(); k++) {
                                                json cord;
                                                auto result = util::convertMaxResToSubResCoord(resolutionInfo,
                                                                                               {pBranch[k].x,
                                                                                                pBranch[k].y,
                                                                                                pBranch[k].z});
                                                cord["x"] = result.x;
                                                cord["y"] = result.y;
                                                cord["z"] = result.z;
                                                offspringsCoorsP.push_back(cord);
                                            }
                                        }
                                    }
                                }
                            }
                        };

                exportParentAndChildPoint(p1Branch, p1, parentsCoorsP1, offspringsCoorsP1);
                exportParentAndChildPoint(p2Branch, p2, parentsCoorsP2, offspringsCoorsP2);

                auto onePointInfo1Result = util::convertMaxResToSubResCoord(resolutionInfo, {p1.x, p1.y, p1.z});
                onePointInfo1["x"] = onePointInfo1Result.x;
                onePointInfo1["y"] = onePointInfo1Result.y;
                onePointInfo1["z"] = onePointInfo1Result.z;
                onePointInfo1["parentsCoors"] = parentsCoorsP1;
                onePointInfo1["offspringsCoors"] = offspringsCoorsP1;

                auto onePointInfo2Result = util::convertMaxResToSubResCoord(resolutionInfo, {p2.x, p2.y, p2.z});
                onePointInfo2["x"] = onePointInfo2Result.x;
                onePointInfo2["y"] = onePointInfo2Result.y;
                onePointInfo2["z"] = onePointInfo2Result.z;
                onePointInfo2["parentsCoors"] = parentsCoorsP2;
                onePointInfo2["offspringsCoors"] = offspringsCoorsP2;

                onePointPairInfo.push_back(onePointInfo1);
                onePointPairInfo.push_back(onePointInfo2);

                jsonObj.push_back(onePointPairInfo);

                std::string str = "0,,,," +
                                  std::to_string(onePointInfo1Result.z)
                                  + "," + std::to_string(onePointInfo1Result.x)
                                  + "," + std::to_string(onePointInfo1Result.y)
                                  + ",0.000,0.000,0.000,314.159,0.000,,,,128,168,255";
                apoOutFile << str << "\n";
                idx++;
            }
        }

        apoOutFile.close();
        jsonOutFile << jsonObj.dump(4);
    }
    jsonOutFile.close();
    std::cout << "Find Point Pair Number: "<<idx<<" \n";
}

QJsonArray ResultWriter::getData(const std::vector<CrossingDetect::KeyPointsType> &keyPointsList,
                           const std::vector<CrossingDetect::BranchesType> &selectedBranchesList){

    for (int i = 0; i < keyPointsList.size(); i++) {
        auto &keyPoints = keyPointsList[i];
        auto &selectedBranches = selectedBranchesList[i];

        QJsonArray infos;
        for (auto &keyPointPair: keyPoints) {
            if (keyPointPair.second) {
                QJsonArray parentsCoorsP1;
                QJsonArray offspringsCoorsP1;

                QJsonArray parentsCoorsP2;
                QJsonArray offspringsCoorsP2;

                QJsonArray onePointPairInfo;
                QJsonObject onePointInfo1;
                QJsonObject onePointInfo2;

                auto [p1, branch1Idx] = keyPointPair.first.first;
                auto [p2, branch2Idx] = keyPointPair.first.second;

                auto &p1Branch = selectedBranches[branch1Idx].first;
                auto &p2Branch = selectedBranches[branch2Idx].first;

                auto exportNumber = ConfigManager::getInstance().exportParentAndChildNodeNumber;

                auto exportParentAndChildPoint =
                    [exportNumber](const std::vector<util::Node> &pBranch,
                                                   util::Node p,
                                                   QJsonArray &parentsCoorsP,
                                                   QJsonArray &offspringsCoorsP
                                                   ) {
                        for (int i = 0; i < pBranch.size(); ++i) {
                            if (pBranch[i].n == p.n) {
                                if (i + 1 > exportNumber) {
                                    for (int j = i - exportNumber + 1; j < i; ++j) {
                                        QJsonObject cord;

                                        cord["x"] = pBranch[j].x;
                                        cord["y"] = pBranch[j].y;
                                        cord["z"] = pBranch[j].z;
                                        parentsCoorsP.append(cord);
                                    }

                                    if (pBranch.size() - i - 1 > exportNumber) {
                                        for (int k = i + 1; k < i + exportNumber + 1; ++k) {
                                            QJsonObject cord;

                                            cord["x"] = pBranch[k].x;
                                            cord["y"] = pBranch[k].y;
                                            cord["z"] = pBranch[k].z;
                                            offspringsCoorsP.append(cord);
                                        }
                                    } else {
                                        for (int k = i + 1; k < pBranch.size(); k++) {
                                            QJsonObject cord;

                                            cord["x"] = pBranch[k].x;
                                            cord["y"] = pBranch[k].y;
                                            cord["z"] = pBranch[k].z;
                                            offspringsCoorsP.append(cord);
                                        }
                                    }
                                } else {
                                    for (int j = 0; j < i; j++) {
                                        QJsonObject cord;

                                        cord["x"] = pBranch[j].x;
                                        cord["y"] = pBranch[j].y;
                                        cord["z"] = pBranch[j].z;
                                        parentsCoorsP.append(cord);
                                    }

                                    if (pBranch.size() - i - 1 > exportNumber) {
                                        for (int k = i + 1; k < i + exportNumber + 1; ++k) {
                                            QJsonObject cord;

                                            cord["x"] = pBranch[k].x;
                                            cord["y"] = pBranch[k].y;
                                            cord["z"] = pBranch[k].z;
                                            offspringsCoorsP.append(cord);
                                        }
                                    } else {
                                        for (int k = i + 1; k < pBranch.size(); k++) {
                                            QJsonObject cord;

                                            cord["x"] = pBranch[k].x;
                                            cord["y"] = pBranch[k].y;
                                            cord["z"] = pBranch[k].z;
                                            offspringsCoorsP.append(cord);
                                        }
                                    }
                                }
                            }
                        }
                    };

                exportParentAndChildPoint(p1Branch, p1, parentsCoorsP1, offspringsCoorsP1);
                exportParentAndChildPoint(p2Branch, p2, parentsCoorsP2, offspringsCoorsP2);

                onePointInfo1["x"] = p1.x;
                onePointInfo1["y"] = p1.y;
                onePointInfo1["z"] = p1.z;
                onePointInfo1["parentsCoors"] = parentsCoorsP1;
                onePointInfo1["offspringsCoors"] = offspringsCoorsP1;

                onePointInfo2["x"] = p2.x;
                onePointInfo2["y"] = p2.y;
                onePointInfo2["z"] = p2.z;
                onePointInfo2["parentsCoors"] = parentsCoorsP2;
                onePointInfo2["offspringsCoors"] = offspringsCoorsP2;

                onePointPairInfo.append(onePointInfo1);
                onePointPairInfo.append(onePointInfo2);

                infos.append(onePointPairInfo);

            }
        }
        return infos;
    }
}

