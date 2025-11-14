// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "helpers/containerUtils.h"
#include "gameTypes/MapCoordinates.h"
#include <limits>
#include <vector>
#include <functional>
#include <map>

class noRoadNode;

namespace dstar
{

// State information stored per node
struct NodeState
{
    // current best-known cost from here to goal
    unsigned g = std::numeric_limits<unsigned>::max();

    // one-step lookahead cost
    unsigned rhs = std::numeric_limits<unsigned>::max();
};

// Node in the priority queue
struct QueueNode
{
    MapPoint nodePos = {0, 0};
    unsigned key = std::numeric_limits<unsigned>::max();
};

// Simple priority queue implementation for D*lite
struct OpenList
{
    std::vector<QueueNode> queue;

    // @todo: we can keep a bitset of dirty nodes to speed up checking for existence
    std::vector<MapPoint> dirty_nodes;

    inline QueueNode Top() const {
        RTTR_Assert(!queue.empty());
        if (queue.size() == 1)
            return queue.front();
        // return best entry
        QueueNode best = queue.front();
        for (const auto& node : queue) {
            if (node.key < best.key)
                best = node;
        }
        return best;
    }

    inline void Remove(const MapPoint& nodePos) {
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->nodePos == nodePos) {
                queue.erase(it);
                return;
            }
        }
    }

    inline void Push(QueueNode node) {
        queue.push_back(node);
    }

    inline void AddDirty(MapPoint pos) {
        // add to dirty nodes if not already present
        if (!helpers::contains(dirty_nodes, pos))
            dirty_nodes.push_back(pos);
    }
};

// Container for per-goal data stored in nodes and RoadPathFinder.
// Value type must be default-constructible.
// Key type must be hashable and comparable.
template<typename T>
class GoalContainer
{
public:
    // @todo: use a more memory-efficient structure if needed (e.g. vector)
    std::map<MapPoint, T, MapPointLess> map;

    inline bool Exists(const MapPoint& goal) const {
        return map.find(goal) != map.end();
    }

    inline T& Get(const MapPoint& goal)
    {
        auto [it, _] = map.emplace(goal, T());
        return it->second;
    }
    
    inline const T& Get(const MapPoint& goal) const
    {
        auto [it, _] = map.emplace(goal, T());
        return it->second;
    }

    inline void Remove(const MapPoint& goal) {
        map.erase(goal);
    }
};

}
