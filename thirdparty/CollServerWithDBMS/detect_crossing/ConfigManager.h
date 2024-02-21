#pragma once

#include <cstddef>
class ConfigManager{
private:
    ConfigManager()= default;

public:
    ConfigManager(ConfigManager&) = delete;
    ConfigManager& operator=(ConfigManager&) = delete;

    static ConfigManager& getInstance(){
        static ConfigManager instance;
        return instance;
    }

    // detect config
    size_t ignoreShortBranchSize = 12;
    float nearestPointDistanceThreshold = 2.0;
    size_t nearestPointParentNodeNumThreshold = 10;
    size_t nearestPointChildNodeNumThreshold = 10;
    float forkPointIgnoreDistanceThreshold = 5 * 1.17;
    float mergeNearestPointOnSameBranchThreshold = 20.0;
    float edgePointIgnoreThreshold = 33.0;

    // export result config
    size_t exportParentAndChildNodeNumber = 24;
};

