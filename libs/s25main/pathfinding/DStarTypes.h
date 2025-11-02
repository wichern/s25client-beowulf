// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "helpers/containerUtils.h"
#include <limits>
#include <unordered_map>
#include <vector>

class noRoadNode;

namespace dstar
{

// Nodes:
//  g per goal
//  rhs per goal
//
// on-delete:
//  remove from open list in U

// Goals:
//  U
//  km
//
// on-delete:
//  remove goal from all nodes

// possible linkage from goal to nodes
//
// #1: U contains MapPoint only, nodes are looked up on demand
//      -> safe, can handle removed nodes
// #2: U contains pointers to nodes
//      -> faster, but need to remove from U on node deletion

// possible linkage from nodes to goals
//
// #1: nodes link to goals via MapPoint
//      -> safe, can handle removed goals
// #2: nodes link to goals via pointers
//      -> faster, but need to remove from nodes on goal deletion

// State information stored per node
struct NodeState
{
    // current best-known cost from here to goal
    unsigned g = std::numeric_limits<unsigned>::max();

    // one-step lookahead cost
    unsigned rhs = std::numeric_limits<unsigned>::max();
};

// Key used for priority queue
struct Key
{
    unsigned k1 = std::numeric_limits<unsigned>::max();
    unsigned k2 = std::numeric_limits<unsigned>::max();

    inline bool operator<(const Key& rhs) const {
        if (k1 == rhs.k1)
            return k2 < rhs.k2;
        return k1 < rhs.k1;
    }
};

// Node in the priority queue
struct QueueNode
{
    noRoadNode* node = nullptr;
    Key key;
};

// Simple priority queue implementation for D*lite
struct OpenList
{
    std::vector<QueueNode> queue;
    std::vector<MapPoint> dirty_nodes;
    unsigned km = 0;

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

    inline void Remove(const QueueNode& node) {
        Remove(node.node);
    }

    inline void Remove(const noRoadNode* node) {
        RTTR_Assert(node);
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->node == node) {
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
template<typename ValueType, typename KeyType = const noRoadNode*>
class GoalContainer
{
public:
    // @todo: use a more memory-efficient structure if needed (e.g. vector)
    mutable std::unordered_map<KeyType, ValueType> map;

    inline bool Exists(KeyType goal) const {
        return map.find(goal) != map.end();
    }

    inline ValueType& Get(KeyType goal)
    {
        auto [it, _] = map.emplace(goal, ValueType());
        return it->second;
    }
    
    inline const ValueType& Get(KeyType goal) const
    {
        auto [it, _] = map.emplace(goal, ValueType());
        return it->second;
    }

    inline void Remove(KeyType goal) {
        map.erase(goal);
    }
};

}
