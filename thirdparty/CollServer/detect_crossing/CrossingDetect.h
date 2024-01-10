#pragma once

#include "utilities.h"
#include <unordered_set>
#include <future>
#include <iostream>

#define ENABLE_DEBUG_MESSAGE 0

class CrossingDetect{
public:
    CrossingDetect();

    using KeyPointsType = std::vector<
                std::pair<
                    std::pair<
                        std::pair<util::Node, int>,
                        std::pair<util::Node, int>
                    >,
                    bool>>;

    using BranchesType = std::vector<std::pair<std::vector<util::Node>, util::BoundingBoxNode>>;

    void initializeNodeData(std::vector<util::Node>& nodes, std::vector<int>& rootNodeIds);

    void generateBranches();

    void selectBranches();

    void generateNearestKeyPoint();

    void removeFromKeyPoint();

    KeyPointsType& getKeyPoint();
    BranchesType& getSelectedBranch();

private:
    std::vector<util::Node> m_Nodes;
    std::vector<int32_t> m_RootNodeIds;

    std::unordered_map<int, util::Node> nodeMap;
    std::unordered_map<int, std::vector<int>> childNodeMap;
    BranchesType branches;
    std::unordered_set<util::Node> branchingNodes;
    BranchesType selectedBranchs;
    KeyPointsType keyPoints;

};
