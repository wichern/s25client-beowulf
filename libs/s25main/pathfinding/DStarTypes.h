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
#include <bitset>
#include <utility> /* std::swap */
#include "gameData/MapConsts.h"

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
    // To speed up the call to Top(), we make sure that the first element in the queue is always the min element.
    std::vector<QueueNode> queue;

    // @todo: we can keep a bitset of dirty nodes to speed up checking for existence
    std::vector<MapPoint> dirty_nodes;

    inline QueueNode Top() {
        RTTR_Assert(!queue.empty());
        return queue.front();
    }

    inline void Remove(const MapPoint& nodePos) {
        for (unsigned i = 0; i < queue.size(); ++i) {
            const auto& node = queue[i];
            if (node.nodePos == nodePos) {
                queue.erase(queue.begin() + i);
                if (i == 0) {
                    unsigned best_key = queue.front().key;
                    unsigned best_idx = 0;
                    for (unsigned i = 1; i < queue.size(); ++i) {
                        const auto& node = queue[i];
                        if (node.key < best_key) {
                            best_key = node.key;
                            best_idx = i;
                        }
                    }
                    if (best_idx != 0) {
                        std::swap(queue[0], queue[best_idx]);
                    }
                }
                return;
            }
        }
    }

    inline void Push(QueueNode node) {
        queue.push_back(node);
        if (node.key < queue.front().key)
            std::swap(queue.front(), queue.back());
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
#if 0
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
#else
struct MapPointHasher {
    std::size_t operator()(MapPoint const& pt) const noexcept {
        static_assert(MAX_MAP_SIZE == 2048);
        return (pt.y << 11) | pt.x;
    }
};

template<typename T>
class GoalContainer
{
public:
    std::unordered_map<MapPoint, T, MapPointHasher> map;

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
#endif

}
