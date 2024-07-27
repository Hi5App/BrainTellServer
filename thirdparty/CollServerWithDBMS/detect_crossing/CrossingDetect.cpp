#include "CrossingDetect.h"
#include "ConfigManager.h"

void CrossingDetect::initializeNodeData(std::vector<util::Node> &nodes, std::vector<int>& rootNodeIds) {
    m_Nodes = std::move(nodes);
    m_RootNodeIds = std::move(rootNodeIds);

    if (m_RootNodeIds.size() != 1) {
        throw std::runtime_error("Root Node Id error!");
    }

    // build nodeMap
    for (auto &node: m_Nodes) {
        nodeMap[node.n] = node;
    }

    // build childNodeMap
    childNodeMap = util::buildChildMap(m_Nodes);

}

void CrossingDetect::generateBranches() {
    util::findBranches(m_RootNodeIds[0], childNodeMap, nodeMap, branches, branchingNodes);

#if ENABLE_DEBUG_MESSAGE == 1 && false
    for (const auto &branch: branches) {
        for (auto& node: branch.first) {
            std::cout << node.n << " ";
        }
        std::cout << std::endl;
    }
#endif

#if ENABLE_DEBUG_MESSAGE == 1
    std::cout << "Branching Nodes:" << std::endl;
    for (auto node : branchingNodes) {
        std::cout << node.n << " ";
    }
    std::cout << std::endl;
#endif
}

void CrossingDetect::selectBranches() {
    for (const auto &branch: branches) {
        if(branch.first.size() > ConfigManager::getInstance().ignoreShortBranchSize){
            selectedBranchs.push_back(branch);
        }
    }
}

void CrossingDetect::generateNearestKeyPoint() {
    auto maxThreads = std::thread::hardware_concurrency();
    std::mutex mutex1;
    std::vector<std::future<void>> futures1;
    for (int cidx1 = 0; cidx1 < selectedBranchs.size(); cidx1++) {
        auto task = [this,&mutex1](int cidx1) {
            auto& selectedBranchs = this->selectedBranchs;
            auto& keyPoints = this->keyPoints;
            KeyPointsType currentKeyPointBetweenTwoBranch;

            for (int cidx2 = cidx1 + 1; cidx2 < selectedBranchs.size(); cidx2++) {
                if (util::boundingBoxIntersect(selectedBranchs[cidx1].second, selectedBranchs[cidx2].second)) {
                    std::span<util::Node> span2 = selectedBranchs[cidx2].first;
                    util::OctreePointCustom octree1(span2, 8);

                    for (int bidx1 = 0; bidx1 < selectedBranchs[cidx1].first.size(); bidx1++) {
                        auto nearstPointVec = octree1.GetNearestNeighbors(selectedBranchs[cidx1].first[bidx1], 1,
                                                                          span2);
                        if (nearstPointVec.empty()) {
                            continue;
                        } else [[likely]] {
                            auto a1 = selectedBranchs[cidx1].first[bidx1];
                            auto a2 = span2[nearstPointVec[0]];
                            auto distance = neuronDistance(a1, a2);
                            if (distance < ConfigManager::getInstance().nearestPointDistanceThreshold
                                && selectedBranchs[cidx1].first.size() > 1
                                && selectedBranchs[cidx2].first.size() > 1) {
                                bool bAddToCollection = true;
                                for (auto& existPoint : currentKeyPointBetweenTwoBranch) {
                                    if(neuronDistance(existPoint.first.second.first, a2) < ConfigManager::getInstance().mergeNearestPointOnSameBranchThreshold){
                                        bAddToCollection = false;
                                        break;
                                    }
                                }
                                if(bAddToCollection) {
                                    currentKeyPointBetweenTwoBranch.push_back({{{a1, cidx1},
                                                                                {a2, cidx2}}, true});
                                }

                            }
                        }
                    }
                }
            }
            {
                std::lock_guard<std::mutex> lock(mutex1);
                for (auto &keypoint: currentKeyPointBetweenTwoBranch) {
                    keyPoints.push_back(keypoint);
                }
            }
        };
        if (futures1.size() >= maxThreads) {
            for (auto& f : futures1) {
                f.get();
            }
            futures1.clear();
        }
        auto result = std::async(std::launch::async, task, cidx1);
        futures1.push_back(std::move(result));
    }
    for (auto& f : futures1) {
        f.get();
    }
    futures1.clear();

#if ENABLE_DEBUG_MESSAGE == 1
    std::cout<<"Before delete:"<<keyPoints.size()<<"\n";
#endif
}

void CrossingDetect::removeFromKeyPoint() {
    auto maxThreads = std::thread::hardware_concurrency();

    auto isParentAndChildHasEnoughNode = [&](
            int p1, int p2, int b1, int b2){
        int p1ParentNum = 0, p2ParentNum = 0;
        int p1ChildNum = 0, p2ChildNum = 0;

        auto& b1Branch = selectedBranchs[b1].first;
        auto& b2Branch = selectedBranchs[b2].first;

        for(int i=0;i < b1Branch.size();i++){
            if(nodeMap[p1].n == b1Branch[i].n){
                p1ParentNum = i;
                p1ChildNum = b1Branch.size() - i - 1;
            }
        }

        for(int i=0;i < b2Branch.size();i++){
            if(nodeMap[p2].n == b2Branch[i].n){
                p2ParentNum = i;
                p2ChildNum = b2Branch.size() - i - 1;
            }
        }

        auto parentThreshold = ConfigManager::getInstance().nearestPointParentNodeNumThreshold;
        auto childThreshold = ConfigManager::getInstance().nearestPointChildNodeNumThreshold;

        if(p1ParentNum >= parentThreshold
           && p2ParentNum >= parentThreshold
           && p1ChildNum >= childThreshold
           && p2ChildNum >= childThreshold){
            return true;
        }else{
            return false;
        }
    };

    std::mutex mutex2;
    std::vector<std::future<void>> futures2;
    for(auto& sp : branchingNodes){
        auto task = [this,&isParentAndChildHasEnoughNode,&mutex2](const util::Node& sp) {
            for (auto &point: keyPoints) {
                auto distance1 = neuronDistance(sp, point.first.first.first);
                auto distance2 = neuronDistance(sp, point.first.second.first);

                {
                    std::lock_guard<std::mutex> lock(mutex2);
                    if(!point.second){
                        continue;
                    }
                }

                auto res = isParentAndChildHasEnoughNode(point.first.first.first.n, point.first.second.first.n,
                                                         point.first.first.second, point.first.second.second);

                float distanceThreshold = ConfigManager::getInstance().forkPointIgnoreDistanceThreshold;

                if(distance1 < distanceThreshold || distance2 < distanceThreshold
                   || point.first.first.first.parent == point.first.second.first.n
                   || point.first.second.first.parent == point.first.first.first.n
                   || !res
                        ){
                    std::lock_guard<std::mutex> lock(mutex2);
                    point.second = false;
                }
            }
        };
        if (futures2.size() >= maxThreads) {
            for (auto& f : futures2) {
                f.get();
            }
            futures2.clear();
        }
        auto result = std::async(std::launch::async, task, sp);
        futures2.push_back(std::move(result));
    }
    for (auto& f : futures2) {
        f.get();
    }
    futures2.clear();
#if ENABLE_DEBUG_MESSAGE == 1
    std::cout<<"After delete:"<<keyPoints.size()<<"\n";
#endif
}

CrossingDetect::KeyPointsType& CrossingDetect::getKeyPoint() {
    return keyPoints;
}

CrossingDetect::BranchesType &CrossingDetect::getSelectedBranch() {
    return selectedBranchs;
}

CrossingDetect::CrossingDetect() {

}
