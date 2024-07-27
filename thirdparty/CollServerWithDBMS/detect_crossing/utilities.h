#pragma once

#include "octree.h"
#include "octree_container.h"
#include <functional>
#include <string>
#include <unordered_set>

using IsEswcType = bool;

namespace util {
    struct Vec3 {
        float x, y, z;
    };

    struct ImageResolutionInfo {
        Vec3 maxRes{1, 1, 1};
        Vec3 subRes{1, 1, 1};
    };

    inline Vec3 convertMaxResToSubResCoord(ImageResolutionInfo info, Vec3 coord) {
        Vec3 result{};
        result.x = coord.x / (info.maxRes.x / info.subRes.x);
        result.y = coord.y / (info.maxRes.y / info.subRes.y);
        result.z = coord.z / (info.maxRes.z / info.subRes.z);
        return result;
    }

    struct Node {
        int n{1}, parent{-1};
        float x{0.0}, y{0.0}, z{0.0};

        std::string getString() {
            //std::string str =
            //	std::to_string(n) + " " +
            //	std::to_string(type) + " " +
            //	std::to_string(x) + " " +
            //	std::to_string(y) + " " +
            //	std::to_string(z) + " " +
            //	std::to_string(radius) + " " +
            //	std::to_string(parent) + " " +
            //	std::to_string(seg_id) + " " +
            //	std::to_string(level) + " " +
            //	std::to_string(mode) + " " +
            //	std::to_string(timestamp) + " " +
            //	std::to_string(feature_value);
            // for apo format zxy
            std::string str = "0, , , , " +
                              std::to_string(z) + ", " + std::to_string(x) + ", " + std::to_string(y) +
                              ", 0.000, 0.000, 0.000, 314.159, 0.000, , , , 128, 168, 255";
            return str;
        }

        bool operator==(const Node &other) const {
            return n == other.n &&
                   parent == other.parent &&
                   x == other.x &&
                   y == other.y &&
                   z == other.z;
        }
    };

    using namespace OrthoTree;
    // User-defined geometrical objects
    using BoundingBoxNode = std::array<Node, 2>;

    // Adaptor
    struct AdaptorBasicsCustom {
        static inline float &point_comp(Node &pt, OrthoTree::dim_type iDimension) {
            switch (iDimension) {
                case 0:
                    return pt.x;
                case 1:
                    return pt.y;
                case 2:
                    return pt.z;
                default:
                    assert(false);
                    return pt.x;
            }
        }

        static constexpr float point_comp_c(Node const &pt, OrthoTree::dim_type iDimension) {
            switch (iDimension) {
                case 0:
                    return pt.x;
                case 1:
                    return pt.y;
                case 2:
                    return pt.z;
                default:
                    assert(false);
                    return pt.x;
            }
        }

        static inline Node &box_min(BoundingBoxNode &box) { return box[0]; }

        static inline Node &box_max(BoundingBoxNode &box) { return box[1]; }

        static constexpr Node const &box_min_c(BoundingBoxNode const &box) { return box[0]; }

        static constexpr Node const &box_max_c(BoundingBoxNode const &box) { return box[1]; }
    };

    using AdaptorCustom = OrthoTree::AdaptorGeneralBase<3, Node, BoundingBoxNode, AdaptorBasicsCustom, float>;

    // Tailored Quadtree objects
    using OctreePointCustom = OrthoTree::OrthoTreePoint<3, Node, BoundingBoxNode, AdaptorCustom, float>;
    using OctreeBoxCustom = OrthoTree::OrthoTreeBoundingBox<3, Node, BoundingBoxNode, AdaptorCustom, float>;


    inline bool boundingBoxIntersect(const BoundingBoxNode &b1, const BoundingBoxNode &b2) {
        return (b1[0].x <= b2[1].x && b1[1].x >= b2[0].x) &&
               (b1[0].y <= b2[1].y && b1[1].y >= b2[0].y) &&
               (b1[0].z <= b2[1].z && b1[1].z >= b2[0].z);
    }

    inline float neuronDistance(const Node &n1, const Node &n2) {
        auto dx = n1.x - n2.x;
        auto dy = n1.y - n2.y;
        auto dz = n1.z - n2.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline std::unordered_map<int, std::vector<int>> buildChildMap(const std::vector<Node> &nodes) {
        std::unordered_map<int, std::vector<int>> childMap;
        for (const auto &node: nodes) {
            childMap[node.parent].push_back(node.n);
        }
        return childMap;
    }

}

template<>
struct std::hash<util::Node> {
    size_t operator()(const util::Node &node) const noexcept {
        size_t h1 = hash<int>()(node.n);
        size_t h2 = hash<int>()(node.parent);
        size_t h3 = hash<float>()(node.x);
        size_t h4 = hash<float>()(node.y);
        size_t h5 = hash<float>()(node.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
    }
};

namespace util {
    inline void findBranches(int root, const std::unordered_map<int, std::vector<int>> &childMap,
                             std::unordered_map<int, util::Node> &nodeMap,
                             std::vector<std::pair<std::vector<Node>, BoundingBoxNode>> &branches,
                             std::unordered_set<Node> &branchingNodes) {
        auto updateBoundingBox = [](
                Node &ref,
                std::pair<std::vector<Node>, BoundingBoxNode> &currentBranch
        ) {
            if (ref.x < currentBranch.second[0].x) {
                currentBranch.second[0].x = ref.x;
            }
            if (ref.y < currentBranch.second[0].y) {
                currentBranch.second[0].y = ref.y;
            }
            if (ref.z < currentBranch.second[0].z) {
                currentBranch.second[0].z = ref.z;
            }

            if (ref.x > currentBranch.second[1].x) {
                currentBranch.second[1].x = ref.x;
            }
            if (ref.y > currentBranch.second[1].y) {
                currentBranch.second[1].y = ref.y;
            }
            if (ref.z > currentBranch.second[1].z) {
                currentBranch.second[1].z = ref.z;
            }
        };

        std::queue<std::pair<int, std::pair<std::vector<Node>, BoundingBoxNode>>> q;
        q.push({root, {std::vector<Node>{},
                       BoundingBoxNode{
                               Node{1, -1,
                                    std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::max(),
                               },
                               Node{1, -1,
                                    -std::numeric_limits<float>::max(),
                                    -std::numeric_limits<float>::max(),
                                    -std::numeric_limits<float>::max(),
                               }
                       }}});

        while (!q.empty()) {
            auto [nodeId, currentBranch] = q.front();
            q.pop();
            auto it = childMap.find(nodeId);
            if (it == childMap.end() || it->second.size() != 1) {
                // Leaf node or branching node found, save the current branch
                if (!currentBranch.first.empty()) {

                    Node lowBounding{1, -1};
                    lowBounding.x = currentBranch.second[0].x - 6 * 1.7;
                    lowBounding.y = currentBranch.second[0].y - 6 * 1.7;
                    lowBounding.z = currentBranch.second[0].z - 6 * 1.7;

                    Node highBounding{1, -1};
                    highBounding.x = currentBranch.second[1].x + 6 * 1.7;
                    highBounding.y = currentBranch.second[1].y + 6 * 1.7;
                    highBounding.z = currentBranch.second[1].z + 6 * 1.7;

                    updateBoundingBox(lowBounding, currentBranch);
                    updateBoundingBox(highBounding, currentBranch);

                    branches.push_back(currentBranch);
                }
                // If branching node, start new branches for each child
                if (it != childMap.end()) {
                    auto nodeInfo = nodeMap[nodeId];
                    branchingNodes.insert(nodeInfo);
                    for (int childId: it->second) {
                        auto ref = std::pair{
                                childId,
                                std::pair{std::vector{nodeInfo}, BoundingBoxNode{
                                        Node{1, -1,
                                             nodeInfo.x,
                                             nodeInfo.y,
                                             nodeInfo.z,
                                        },
                                        Node{1, -1,
                                             nodeInfo.x,
                                             nodeInfo.y,
                                             nodeInfo.z,
                                        }
                                }}};
                        q.push(ref);
                    }
                }
            } else {
                // Continue along the single child
                auto ref = nodeMap[nodeId];
                currentBranch.first.push_back(ref);
                updateBoundingBox(ref, currentBranch);
                q.push({it->second[0], currentBranch});
            }
        }
    }
}
