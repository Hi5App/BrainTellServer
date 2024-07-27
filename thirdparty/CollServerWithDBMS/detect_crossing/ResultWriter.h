#pragma once

#include "json.hpp"
#include "CrossingDetect.h"
#include "ConfigManager.h"
#include <fstream>
#include <QJsonObject>
#include <QJsonArray>
#include "utilities.h"

using namespace nlohmann;

class ResultWriter {
public:
    ResultWriter(const std::string &jsonFilePath, const std::string &apoFilePath);

    void writeData(const std::vector<CrossingDetect::KeyPointsType> &keyPointsList,
                   const std::vector<CrossingDetect::BranchesType> &selectedBranchesList,
                   std::vector<util::ImageResolutionInfo> resolutionInfoList);

    QJsonArray getData(const std::vector<CrossingDetect::KeyPointsType> &keyPointsList,
                 const std::vector<CrossingDetect::BranchesType> &selectedBranchesList);

private:
    std::string m_JsonFilePath;
    std::string m_ApoFilePath;

};

