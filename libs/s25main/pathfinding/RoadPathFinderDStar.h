// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "helpers/EnumRange.h"
#include "pathfinding/OpenListVector.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noRoadNode.h"
#include "gameTypes/MapCoordinates.h"
#include <iostream>
#include <utility>

#include "AsciiMap.h"

namespace dstarlite {

static const unsigned g_dstarNodeCountLimit = 10U; // how many nodes can a vector contain?
static const unsigned g_dstarNodeAgeLimit = 1000U; // how old can a node be?

struct Key
{
    unsigned k1; // f = g + h ... same as in A* we use this value to find the next best node
    unsigned k2; // g         ... in case k1 is the same for two nodes, we take the one with the better g value
};

bool operator<(const Key& l, const Key& r);

Key CalculateKey(const Node& node, unsigned h);

struct GetKeyFromSecond
{
    template<typename T>
    static inline Key GetValue(const T& el)
    {
        return el.second;
    }
};

template<class T_AdditionalCosts, class T_SegmentConstraints>
class Search
{
    const GameWorldBase& map_;
    const noRoadNode& goal_;
    const unsigned gf_;
    const T_AdditionalCosts& addCosts_;
    const T_SegmentConstraints& isSegmentAllowed_;

public:
    Search(const GameWorldBase& map, const noRoadNode& goal, unsigned gf, const T_AdditionalCosts addCosts,
           const T_SegmentConstraints isSegmentAllowed)
        : map_(map), goal_(goal), gf_(gf), addCosts_(addCosts), isSegmentAllowed_(isSegmentAllowed)
    {}

    bool ComputeShortestPath(const noRoadNode& start, unsigned maxCost);
    // std::pair<const noRoadNode*, RoadPathDirection> GetMinCostSuccessor(const noRoadNode& node);

    unsigned GetMinCostSuccessor(const noRoadNode& node, const noRoadNode** nextNode = nullptr,
                                 Direction* nextDir = nullptr);
    Node& GetNode(const noRoadNode& node) const;

private:
    unsigned Heuristic(const noRoadNode& node) const;

    template<class T_Callback>
    void VisitNeighbours(const noRoadNode& node, T_Callback cb);
};

#define TRACE_DSTAR

template<class T_AdditionalCosts, class T_SegmentConstraints>
bool Search<T_AdditionalCosts, T_SegmentConstraints>::ComputeShortestPath(const noRoadNode& start, unsigned maxCost)
{
#ifdef TRACE_DSTAR
    AsciiMap debugMap(map_);
#endif
    static_cast<void>(maxCost); // @todo: abort early

    static OpenListVector<std::pair<const noRoadNode*, Key>, Key, GetKeyFromSecond> todo;
    todo.clear();

    // Insert goal as first node into the queue
    Node& goalDnode = GetNode(goal_);
    goalDnode.rhs = 0;

    const Key goalKey = {Heuristic(goal_), 0U};
    todo.push({&goal_, goalKey});
#ifdef TRACE_DSTAR
    std::cout << "push(" << goal_.GetPos().x << ":" << goal_.GetPos().y << " [" << goalKey.k1 << "," << goalKey.k2
              << "])" << std::endl;
#endif

    Node& startDnode = GetNode(start);
    // const Key startKey = CalculateKey(startDnode, Heuristic(start));

    while(!todo.empty())
    {
#ifdef TRACE_DSTAR
        debugMap.clear();
        debugMap.drawPlayer(0);
        debugMap.drawDStar(0, goal_.GetPos());
        debugMap.write();
#endif

        auto bestTuple = todo.pop();
        const noRoadNode& best = *bestTuple.first;
        Key bestKey = bestTuple.second;

#ifdef TRACE_DSTAR
        std::cout << "pop(" << best.GetPos().x << "," << best.GetPos().y << " [" << bestKey.k1 << "," << bestKey.k2
                  << "])" << std::endl;
#endif

        // @todo: Explain abort conditions
        if(!(bestKey < CalculateKey(startDnode, Heuristic(start)) || startDnode.rhs > startDnode.g))
            break;

        // recalc key of best node
        Node& bestDnode = GetNode(best);
        Key bestKeyNew = CalculateKey(bestDnode, Heuristic(best));

        if(bestKey < bestKeyNew)
        {
            todo.push({&best, bestKeyNew});
#ifdef TRACE_DSTAR
            std::cout << "push(" << best.GetPos().x << ":" << best.GetPos().y << " [" << bestKeyNew.k1 << ","
                      << bestKeyNew.k2 << "])" << std::endl;
#endif
        } else if(bestDnode.g > bestDnode.rhs)
        {
            bestDnode.g = bestDnode.rhs;

            VisitNeighbours(
              best, [this, &best, &bestDnode](const noRoadNode& neighbour, const RoadSegment& /*route*/, Direction /*dir*/) {
                  auto& neighbourDnode = GetNode(neighbour);

                  // UpdateVertex
                  if (&neighbour != &goal_) {
                    neighbourDnode.rhs = GetMinCostSuccessor(neighbour);
                  }

                  if(neighbourDnode.g != neighbourDnode.rhs)
                  {
                      Key key = CalculateKey(neighbourDnode, Heuristic(neighbour));
                      todo.push({&neighbour, key});
#ifdef TRACE_DSTAR
                      std::cout << "push(" << neighbour.GetPos().x << ":" << neighbour.GetPos().y << " [" << key.k1
                                << "," << key.k2 << "])" << std::endl;
#endif
                  }
              });
        } else
        {
            unsigned g_old = bestDnode.g;
            bestDnode.g = std::numeric_limits<unsigned>::max();

            VisitNeighbours(best, [this, &best, &bestDnode, g_old](const noRoadNode& neighbour,
                                                                   const RoadSegment& route, Direction dir) {
                auto& neighbourDnode = GetNode(neighbour);

                if(neighbourDnode.rhs == (route.GetLength() + addCosts_(best, dir) + g_old))
                {
                    if(&neighbour != &goal_)
                    {
                        neighbourDnode.rhs = GetMinCostSuccessor(neighbour);
                    }
                }

                // UpdateVertex
                if(neighbourDnode.g != neighbourDnode.rhs)
                {
                    Key key = CalculateKey(neighbourDnode, Heuristic(neighbour));
                    todo.push({&neighbour, key});
#ifdef TRACE_DSTAR
                    std::cout << "push(" << neighbour.GetPos().x << ":" << neighbour.GetPos().y << " [" << key.k1 << ","
                              << key.k2 << "])" << std::endl;
#endif
                }
            });
        }
    }

#ifdef TRACE_DSTAR
        std::cout << "startDnode.g == " << startDnode.g << " (max: " << maxCost << ")" << std::endl;
#endif
    return startDnode.g <= maxCost;
}

template<class T_AdditionalCosts, class T_SegmentConstraints>
template<class T_Callback>
void Search<T_AdditionalCosts, T_SegmentConstraints>::VisitNeighbours(const noRoadNode& node, T_Callback cb)
{
    // Loop over all predecessors
    const helpers::EnumArray<RoadSegment*, Direction>& routes = node.getRoutes();
    for(const auto dir : helpers::EnumRange<Direction>{})
    {
        const auto* route = routes[dir];
        if(!route)
            continue;

        // Check the 2 flags, one is the current node, so we need the other
        noRoadNode* neighbour = route->GetF1();
        if(neighbour == &node)
            neighbour = route->GetF2();

        // No paths over buildings (except if it is the goal)
        if(dir == Direction::NorthWest && neighbour != &goal_)
        {
            // Flags and harbors are allowed
            const GO_Type got = neighbour->GetGOT();
            if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                continue;
        }

        // evtl verboten?
        if(!isSegmentAllowed_(*route))
            continue;

        cb(*neighbour, *route, dir);
    }
}

template<class T_AdditionalCosts, class T_SegmentConstraints>
unsigned Search<T_AdditionalCosts, T_SegmentConstraints>::GetMinCostSuccessor(const noRoadNode& node,
                                                                              const noRoadNode** nextNode,
                                                                              Direction* nextDir)
{
    unsigned minCost = std::numeric_limits<unsigned>::max();

    VisitNeighbours(
      node, [this, &node, &minCost, &nextNode, &nextDir](const noRoadNode& n, const RoadSegment& route, Direction dir) {
          const unsigned cost = route.GetLength() + addCosts_(node, dir);

          auto& nDnode = GetNode(n);
          const unsigned rhs = cost + nDnode.g;

          if(rhs < minCost)
          {
              if(nextNode)
                  *nextNode = &n;
              if(nextDir)
                  *nextDir = dir;
              minCost = rhs;
          }
      });

    return minCost;
}

template<class T_AdditionalCosts, class T_SegmentConstraints>
Node& Search<T_AdditionalCosts, T_SegmentConstraints>::GetNode(const noRoadNode& node) const
{
    auto oldest_it = node.dstar_.begin();

    for(auto it = node.dstar_.begin(); it != node.dstar_.end(); ++it)
    {
        if(it->goalPos == goal_.GetPos())
        {
            it->visit_gf = gf_;
            return *it;
        }

        // while we're here, remove outdated nodes
        if(it->visit_gf < g_dstarNodeAgeLimit)
        {
            it = node.dstar_.erase(it);
            continue;
        }

        if(oldest_it->visit_gf > it->visit_gf)
        {
            oldest_it = it;
        }
    }

    // node not yet on roadNode.

    // remove LRU node if the vector is already too big.
    if(node.dstar_.size() >= g_dstarNodeCountLimit)
    {
        node.dstar_.erase(oldest_it);
    }

    // add new node
    unsigned g = std::numeric_limits<unsigned>::max();
    unsigned rhs = std::numeric_limits<unsigned>::max();
    unsigned k_m = 0;
    Node dnode = {goal_.GetPos(), g, rhs, k_m, gf_};
    node.dstar_.push_back(dnode);
    return node.dstar_.back();
}

template<class T_AdditionalCosts, class T_SegmentConstraints>
unsigned Search<T_AdditionalCosts, T_SegmentConstraints>::Heuristic(const noRoadNode& node) const
{
    return map_.CalcDistance(node.GetPos(), goal_.GetPos());
}

} // namespace dstarlite
