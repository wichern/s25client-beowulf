// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <limits>
#include <unordered_map>
#include <vector>

class noRoadNode;

namespace dstar
{

struct NodeState
{
    unsigned g = std::numeric_limits<unsigned>::max();// current best-known cost from here to goal
    unsigned rhs = std::numeric_limits<unsigned>::max(); // one-step lookahead cost
};

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

struct QueueNode
{
    noRoadNode* node = nullptr;
    Key key;
};

struct OpenList
{
    std::vector<QueueNode> queue;
    std::vector<const noRoadNode*> dirty_nodes;
    unsigned km = 0;

    QueueNode Top() const {
        RTTR_Assert(!queue.empty());
        if (queue.size() == 1)
            return queue.front();
        const QueueNode* best = &queue.front();
        for (const auto& node : queue) {
            if (node.key < best->key)
                best = &node;
        }
        return *best;
    }

    void Remove(const QueueNode& node) {
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->node == node.node) {
                queue.erase(it);
                return;
            }
        }
        RTTR_Assert(false); // node not found
    }

    void Remove(const noRoadNode& node) {
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->node == &node) {
                queue.erase(it);
                return;
            }
        }
        //RTTR_Assert(false); // node not found
    }

    void Push(const QueueNode& node) {
        queue.push_back(node);
    }

    void AddDirty(const noRoadNode& node) {
        // @todo: The road node may not exist anymore (e.g. when it was marked dirty due to destruction)
        // add to dirty nodes if not already present
        for (const auto* n : dirty_nodes) {
            if (n == &node)
                return;
        }
        dirty_nodes.push_back(&node);
    }
};

template<typename T>
class GoalContainer
{
public:
    mutable std::unordered_map<const noRoadNode*, T> map;

    inline bool Exists(const noRoadNode& goal) const {
        return map.find(&goal) != map.end();
    }

    inline T& Get(const noRoadNode& goal)
    {
        auto [it, _] = map.emplace(&goal, T());
        return it->second;
    }
    
    inline const T& Get(const noRoadNode& goal) const
    {
        auto [it, _] = map.emplace(&goal, T());
        return it->second;
    }
};

}
