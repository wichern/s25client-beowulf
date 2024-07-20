// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RoadPathFinder.h"
#include "EventManager.h"
#include "RttrForeachPt.h"
#include "buildings/nobHarborBuilding.h"
#include "pathfinding/OpenListPrioQueue.h"
#include "pathfinding/OpenListVector.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noRoadNode.h"
#include "gameData/GameConsts.h"
#include "s25util/Log.h"

/// Comparison operator for road nodes that returns true if lhs > rhs (descending order)
struct RoadNodeComperatorGreater
{
    bool operator()(const noRoadNode* const lhs, const noRoadNode* const rhs) const
    {
        if(lhs->estimate == rhs->estimate)
        {
            // Wenn die Wegkosten gleich sind, vergleichen wir die Koordinaten, da wir für std::set eine streng
            // monoton steigende Folge brauchen
            return (lhs->GetObjId() > rhs->GetObjId());
        }

        return (lhs->estimate > rhs->estimate);
    }
};

using QueueImpl = OpenListPrioQueue<const noRoadNode*, RoadNodeComperatorGreater>;
using VecImpl = OpenListVector<const noRoadNode*>;
VecImpl todo;

OpenListVector<std::pair<const noRoadNode*, noRoadNode::DStarNode::Key>, noRoadNode::DStarNode::Key, GetKeyFromSecond> todoDLite;

// Namespace with all functors usable as additional cost functors
namespace AdditonalCosts {
struct None
{
    unsigned operator()(const noRoadNode&, const Direction) const { return 0; }
};

struct Carrier
{
    unsigned operator()(const noRoadNode& curNode, const Direction nextDir) const
    {
        // Add costs for busy carriers to allow alternative routes
        return curNode.GetPunishmentPoints(nextDir);
    }
};
} // namespace AdditonalCosts

// Namespace with all functors usable as segment constraint functors
namespace SegmentConstraints {
struct None
{
    bool operator()(const RoadSegment&) const { return true; }
};

/// Disallows a specific road segment
struct AvoidSegment
{
    const RoadSegment* const forbiddenSeg_;
    AvoidSegment(const RoadSegment* const forbiddenSeg) : forbiddenSeg_(forbiddenSeg) {}

    bool operator()(const RoadSegment& segment) const { return forbiddenSeg_ != &segment; }
};

/// Disallows a specific road type
template<RoadType T_roadType>
struct AvoidRoadType
{
    bool operator()(const RoadSegment& segment) const { return segment.GetRoadType() != T_roadType; }
};

/// Combines 2 functors by returning true only if both of them return true
/// Can be chained
template<class T_Func1, class T_Func2>
struct And : private T_Func1, private T_Func2
{
    using Func1 = T_Func1;
    using Func2 = T_Func2;

    And() : Func1(), Func2() {}

    template<typename T>
    And(const T& p1) : Func1(p1), Func2()
    {}

    template<typename T, typename U>
    And(const T& p1, const U& p2) : Func1(p1), Func2(p2)
    {}

    And(const Func2& f2) : Func1(), Func2(f2) {}

    bool operator()(const RoadSegment& segment) const
    {
        return Func1::operator()(segment) && Func2::operator()(segment);
    }
};
} // namespace SegmentConstraints


namespace dstarlite {

static const unsigned g_dstarNodeCountLimit = 10;  // how many nodes can a vector contain?
static const unsigned g_dstarNodeAgeLimit = 1000;  // how old can a node be?

// Get the node or create a new one.
// If number of nodes exceeds threshold, we remove the lru node.
static noRoadNode::DStarNode& GetNode(const noRoadNode& node, const MapPoint& dest, unsigned gf)
{
    auto oldest_it = node.dstar_.begin();

    for (auto it = node.dstar_.begin(); it != node.dstar_.end(); ++it) {
        if (it->dest == dest) {
            it->visit_gf = gf;
            return *it;
        }
        // while we're here, remove outdated nodes
        if (it->visit_gf < g_dstarNodeAgeLimit) {
            it = node.dstar_.erase(it);
            continue;
        }

        if (oldest_it->visit_gf > it->visit_gf) {
            oldest_it = it;
        }
    }

    // node not yet on roadNode.

    // remove LRU node if the vector is already too big.
    if (node.dstar_.size() >= g_dstarNodeCountLimit) {
        node.dstar_.erase(oldest_it);
    }

    // add new node
    unsigned g = std::numeric_limits<unsigned>::max();
    unsigned rhs = std::numeric_limits<unsigned>::max();
    unsigned k_m = 0;
    //noRoadNode::DStarNode::Key k = {0U, 0U};
    noRoadNode::DStarNode dnode = { dest, g, rhs, k_m, gf /*, k*/ };
    node.dstar_.push_back(dnode);
    return node.dstar_.back();
}

}

template<class T_AdditionalCosts, class T_SegmentConstraints>
bool RoadPathFinder::FindPathImplDStarLite(const noRoadNode& start, const noRoadNode& goal, const unsigned max,
                                  const T_AdditionalCosts addCosts, const T_SegmentConstraints isSegmentAllowed,
                                  unsigned* const length, RoadPathDirection* const firstDir,
                                  MapPoint* const firstNodePos)
{
/*
The pseudocode uses the following functions to manage the priority queue: 
- U.Top() returns a vertex with the smallest priority of all vertices in priority queue U.
- U.TopKey() returns the smallest priority of all vertices in priority queue U. (If U is empty, then U.TopKey() returns W X ZY[X;\ .)
- U.Pop() deletes the vertex with the smallest priority in priority queue U and returns the vertex.
- U.Insert(s, k) inserts vertex s into priority queue U with priority k.
- U.Update(s, k) changes the priority of vertex s in priority queue U to k. (It does nothing if the current priority of vertex s already equals k)
- U.Remove(s) removes vertex s from priority queue U.
*/
    // k1, k2 ... Key value calcuated from g and rhs
    // k_m ... ?

    static_cast<void>(max); // @todo: Respect max cost
    static_cast<void>(length); // @todo: store best cost in length
    static_cast<void>(firstDir); // @todo: store first direction in firstDir
    static_cast<void>(firstNodePos); // @todo: store first node pos in firstNodePos

    unsigned currentGf = gwb_.GetEvMgr().GetCurrentGF();

    // ------------------------------------------------------------------------
    // Initialize()

    todoDLite.clear();

    // Insert goal as first node into the queue
    const noRoadNode::DStarNode::Key goal_key = { gwb_.CalcDistance(start.GetPos(), goal.GetPos()), 0U };
    todoDLite.push({&goal, goal_key});

    noRoadNode::DStarNode& start_dnode = dstarlite::GetNode(start, goal.GetPos(), currentGf);
    const noRoadNode::DStarNode::Key start_key = start_dnode.CalculateKey(gwb_.CalcDistance(start.GetPos(), goal.GetPos()));

    // ------------------------------------------------------------------------
    // ComputeShortestPath()

    while (!todoDLite.empty()) {
        const noRoadNode* best_node;
        noRoadNode::DStarNode::Key best_key;
        std::tie(best_node, best_key) = todoDLite.pop();
        noRoadNode::DStarNode& best_dnode = dstarlite::GetNode(*best_node, goal.GetPos(), currentGf);

        // @todo: Explain abort conditions
        if (!(best_key < start_key || start_dnode.rhs > start_dnode.g))
            break;

        // recalc key of best node
        noRoadNode::DStarNode::Key best_key_new = best_dnode.CalculateKey(gwb_.CalcDistance(best_node->GetPos(), goal.GetPos()));

        if (best_key < best_key_new) {
            todoDLite.push({best_node, best_key_new});
        } else if (best_dnode.g > best_dnode.rhs) {
            best_dnode.g = best_dnode.rhs;

            // Loop over all predecessors
            const helpers::EnumArray<RoadSegment*, Direction>& routes = best_node->getRoutes();
            for(const auto dir : helpers::EnumRange<Direction>{})
            {
                const auto* route = routes[dir];
                if(!route)
                    continue;

                // Check the 2 flags, one is the current node, so we need the other
                noRoadNode* neighbour = route->GetF1();
                if(neighbour == best_node)
                    neighbour = route->GetF2();

                // No paths over buildings
                if(dir == Direction::NorthWest && neighbour != &goal)
                {
                    // Flags and harbors are allowed
                    const GO_Type got = neighbour->GetGOT();
                    if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                        continue;
                }

                // evtl verboten?
                if(!isSegmentAllowed(*route))
                    continue;

                auto& neighbour_dnode = dstarlite::GetNode(*neighbour, goal.GetPos(), currentGf);
                if (neighbour != &goal) {
                    const unsigned cost = route->GetLength() + addCosts(*neighbour, dir);
                    neighbour_dnode.rhs = std::min(neighbour_dnode.rhs, cost + best_dnode.g);
                }
                // UpdateVertex
                if (neighbour_dnode.g != neighbour_dnode.rhs) {
                    todoDLite.push({neighbour, neighbour_dnode.CalculateKey(gwb_.CalcDistance(neighbour->GetPos(), goal.GetPos()))});
                }
            }
        } else {
            unsigned g_old = best_dnode.g;
            best_dnode.g = std::numeric_limits<unsigned>::max();

            // Loop over all predecessors
            const helpers::EnumArray<RoadSegment*, Direction>& routes = best_node->getRoutes();
            for(const auto dir : helpers::EnumRange<Direction>{})
            {
                const auto* route = routes[dir];
                if(!route)
                    continue;

                // Check the 2 flags, one is the current node, so we need the other
                noRoadNode* neighbour = route->GetF1();
                if(neighbour == best_node)
                    neighbour = route->GetF2();

                // No paths over buildings
                if(dir == Direction::NorthWest && neighbour != &goal)
                {
                    // Flags and harbors are allowed
                    const GO_Type got = neighbour->GetGOT();
                    if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                        continue;
                }

                // evtl verboten?
                if(!isSegmentAllowed(*route))
                    continue;

                auto& neighbour_dnode = dstarlite::GetNode(*neighbour, goal.GetPos(), currentGf);
                const unsigned cost = route->GetLength() + addCosts(*best_node, dir);
                if (neighbour_dnode.rhs == (cost + g_old)) {
                    if (neighbour != &goal) {
                        // set neighbour_dnode.rhs to the smallest cost of all successors

                        unsigned lowest_rhs = std::numeric_limits<unsigned>::max();

                        // Loop over all successors (same es predecessors, but excludes curret node)
                        const helpers::EnumArray<RoadSegment*, Direction>& routes_n = neighbour->getRoutes();
                        for(const auto dir : helpers::EnumRange<Direction>{})
                        {
                            const auto* route_n = routes_n[dir];
                            if(!route_n)
                                continue;

                            // Check the 2 flags, one is the current node, so we need the other
                            noRoadNode* neighbour_n = route_n->GetF1();
                            if(neighbour_n == neighbour)
                                neighbour_n = route_n->GetF2();

                            if (neighbour_n == best_node)
                                continue;

                            // No paths over buildings
                            if(dir == Direction::NorthWest && neighbour != &goal)
                            {
                                // Flags and harbors are allowed
                                const GO_Type got = neighbour->GetGOT();
                                if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                                    continue;
                            }

                            // evtl verboten?
                            if(!isSegmentAllowed(*route))
                                continue;

                            noRoadNode::DStarNode& neighbour_n_dnode = dstarlite::GetNode(*neighbour_n, goal.GetPos(), currentGf);
                            const unsigned rhs = route_n->GetLength() + addCosts(*neighbour, dir) + neighbour_n_dnode.g;
                            lowest_rhs = std::min(lowest_rhs, rhs);
                        }

                        RTTR_Assert(lowest_rhs != std::numeric_limits<unsigned>::max());
                        neighbour_dnode.rhs = lowest_rhs;
                    }
                }

                // UpdateVertex
                if (neighbour_dnode.g != neighbour_dnode.rhs) {
                    todoDLite.push({neighbour, neighbour_dnode.CalculateKey(gwb_.CalcDistance(neighbour->GetPos(), goal.GetPos()))});
                }
            }
        }
    }





#if 0
    // http://idm-lab.org/bib/abstracts/papers/aaai02b.pdf
    // Initialize()
    /* {02} */ todoDLite.clear();
    /* {03} */ unsigned k_m = 0;
    /* {04} is done by dstarlite::GetNode setting every new node to g = rhs = MAX */
    noRoadNode::DStarNode& dnode_goal = dstarlite::GetNode(goal, goal.GetPos(), currentGf);  // @todo: does the initial node have to be stored in the road node?
    /* {05} */ dnode_goal.rhs = 0;
    noRoadNode::DStarNode::Key k = { gwb_.CalcDistance(start.GetPos(), goal.GetPos()), 0U };
    /* {06} */ todoDLite.push({&goal, k});

    // ComputeShortestPath()
    
    /*
     * Check if the top element in U is smaller than start.key OR start.rhs > start.g (?)
     */

    noRoadNode::DStarNode& dnode_start = dstarlite::GetNode(start, goal.GetPos(), currentGf);
    /* {10} */ while (true) {
        /* {11} */ auto u = todoDLite.pop();
        if (dnode_start.CalculateKey(gwb_.CalcDistance(start.GetPos(), start.GetPos())) < u.second
            && dnode_start.g < dnode_start.rhs)   // conditions for {10}
            break;
        /* {12} */ noRoadNode::DStarNode::Key k_old = u.second;
        auto& u_dnode = dstarlite::GetNode(*u.first, start.GetPos(), currentGf);
        /* {13} */ noRoadNode::DStarNode::Key k_new = u_dnode.CalculateKey(gwb_.CalcDistance(u.first->GetPos(), start.GetPos()));
        /* {14} */ if (k_old < k_new) {
            /* {15} */ u.second = k_new; todoDLite.push(u);
        /* {16} */ } else if (u_dnode.g > u_dnode.rhs) {
                /* {17} */ u_dnode.g = u_dnode.rhs;
                /* {18} */ // we do not remove but push in else case instead, because pop() already removed it
                const auto routes = u.first->getRoutes();
                /* {19} */ for(const auto dir : helpers::EnumRange<Direction>{}) {
                    const auto* route = routes[dir];
                    if(!route)
                        continue;
                    // Check the 2 flags, one is the current node, so we need the other
                    noRoadNode* s = route->GetF1();
                    if(s == u.first)
                        s = route->GetF2();
                    const unsigned cost = route->GetLength() + addCosts(*s, dir);
                    auto& s_dnode = dstarlite::GetNode(*s, start.GetPos(), currentGf);
                    /* {20} */ if (s != &goal) { s_dnode.rhs = std::min(s_dnode.rhs, cost + s_dnode.g); }
                    /* {21} */ todoDLite.push(u);
                }
            /* {22} */ } else {
                /* {23} */ unsigned g_old = u_dnode.g;
                u_dnode.g = std::numeric_limits<unsigned>::max();
        }
    }

#endif

    return true;
}

/// Wegfinden ( A* ), O(v lg v) --> Wegfindung auf Stra�en
template<class T_AdditionalCosts, class T_SegmentConstraints>
bool RoadPathFinder::FindPathImpl(const noRoadNode& start, const noRoadNode& goal, const unsigned max,
                                  const T_AdditionalCosts addCosts, const T_SegmentConstraints isSegmentAllowed,
                                  unsigned* const length, RoadPathDirection* const firstDir,
                                  MapPoint* const firstNodePos)
{
    if(&start == &goal)
    {
        // Path where start==goal should never happen
        RTTR_Assert(false);
        LOG.write("WARNING: Bug detected (GF: %u). Please report this with the savegame and replay (Start==Goal in "
                  "pathfinding %u,%u)\n")
          % gwb_.GetEvMgr().GetCurrentGF() % unsigned(start.GetX()) % unsigned(start.GetY());
        // But for now we assume it to be valid and return (kind of) correct values
        if(length)
            *length = 0;
        if(firstDir)
            *firstDir = RoadPathDirection::None;
        if(firstNodePos)
            *firstNodePos = start.GetPos();
        return true;
    }

    // increase current_visit_on_roads, so we don't have to clear the visited-states at every run
    currentVisit++;

    // if the counter reaches its maximum, tidy up
    if(currentVisit == std::numeric_limits<unsigned>::max())
    {
        RTTR_FOREACH_PT(MapPoint, gwb_.GetSize())
        {
            auto* const node = gwb_.GetSpecObj<noRoadNode>(pt);
            if(node)
                node->last_visit = 0;
        }
        currentVisit = 1;
    }

    // Add start node
    todo.clear();

    const MapPoint goalPos = goal.GetPos();
    start.targetDistance = gwb_.CalcDistance(start.GetPos(), goalPos);
    start.estimate = start.targetDistance;
    start.last_visit = currentVisit;
    start.prev = nullptr;
    start.cost = 0;
    start.dir_ = RoadPathDirection::None;

    todo.push(&start);

    while(!todo.empty())
    {
        // Get node with current least estimate
        const noRoadNode& best = *todo.pop();

        // Reached goal
        if(&best == &goal)
        {
            if(length)
                *length = best.cost;

            // Backtrace to get the last node that is not the start node (has a prev node) --> Next node from start on
            // path
            const noRoadNode* firstNode = &best;
            while(firstNode->prev != &start)
            {
                firstNode = firstNode->prev;
            }

            if(firstDir)
                *firstDir = firstNode->dir_;

            if(firstNodePos)
                *firstNodePos = firstNode->GetPos();

            // Done, path found
            return true;
        }

        const helpers::EnumArray<RoadSegment*, Direction>& routes = best.getRoutes();
        const noRoadNode* prevNode = best.prev;

        // Nachbarflagge bzw. Wege in allen 6 Richtungen verfolgen
        for(const auto dir : helpers::EnumRange<Direction>{})
        {
            const auto* route = routes[dir];
            if(!route)
                continue;

            // Check the 2 flags, one is the current node, so we need the other
            noRoadNode* neighbour = route->GetF1();
            if(neighbour == &best)
                neighbour = route->GetF2();

            // this eliminates 1/6 of all nodes and avoids cost calculation and further checks,
            if(neighbour == prevNode)
                continue;

            // No paths over buildings
            if(dir == Direction::NorthWest && neighbour != &goal)
            {
                // Flags and harbors are allowed
                const GO_Type got = neighbour->GetGOT();
                if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                    continue;
            }

            // evtl verboten?
            if(!isSegmentAllowed(*route))
                continue;

            unsigned cost = best.cost + route->GetLength();
            cost += addCosts(best, dir);

            if(cost > max)
                continue;

            // Was node already visited?
            if(neighbour->last_visit == currentVisit)
            {
                // Update node if costs are lower
                if(cost < neighbour->cost)
                {
                    neighbour->cost = cost;
                    neighbour->estimate = neighbour->targetDistance + cost;
                    neighbour->prev = &best;
                    neighbour->dir_ = toRoadPathDirection(dir);
                    todo.rearrange(neighbour);
                }
            } else
            {
                // Not visited yet -> Add to list
                neighbour->cost = cost;
                neighbour->targetDistance = gwb_.CalcDistance(neighbour->GetPos(), goalPos);
                neighbour->estimate = neighbour->targetDistance + cost;
                neighbour->last_visit = currentVisit;
                neighbour->prev = &best;
                neighbour->dir_ = toRoadPathDirection(dir);

                todo.push(neighbour);
            }
        }

        // For harbors also consider ship connections
        if(best.GetGOT() != GO_Type::NobHarborbuilding)
            continue;
        for(const auto& sc : static_cast<const nobHarborBuilding&>(best).GetShipConnections())
        {
            unsigned cost = best.cost + sc.way_costs;

            if(cost > max)
                continue;

            noRoadNode& dest = *sc.dest;
            // Was node already visited?
            if(dest.last_visit == currentVisit)
            {
                // Update node if costs are lower
                if(cost < dest.cost)
                {
                    dest.cost = cost;
                    dest.estimate = dest.targetDistance + cost;
                    dest.prev = &best;
                    dest.dir_ = RoadPathDirection::Ship;
                    todo.rearrange(&dest);
                }
            } else
            {
                // Not visited yet -> Add to list
                dest.cost = cost;
                dest.targetDistance = gwb_.CalcDistance(dest.GetPos(), goalPos);
                dest.estimate = dest.targetDistance + cost;
                dest.last_visit = currentVisit;
                dest.prev = &best;
                dest.dir_ = RoadPathDirection::Ship;

                todo.push(&dest);
            }
        }
    }

    // Liste leer und kein Ziel erreicht --> kein Weg
    return false;
}

#define ROAD_PATH_FINDER_BOTH
// #define ROAD_PATH_FINDER_A
// #define ROAD_PATH_FINDER_D

bool RoadPathFinder::FindPath(const noRoadNode& start, const noRoadNode& goal, const bool wareMode, const unsigned max,
                              const RoadSegment* const forbidden, unsigned* const length,
                              RoadPathDirection* const firstDir, MapPoint* const firstNodePos)
{
    RTTR_Assert(length || firstDir || firstNodePos); // If none of them is set use the \ref PathExist function!

#if defined(ROAD_PATH_FINDER_BOTH)
    // We add the new D*-lite search here so that we can compare the results.


    if(wareMode)
    {
        if(forbidden) {
            return FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::AvoidSegment(forbidden), length, firstDir, firstNodePos);
        } else {
            RoadPathDirection firstDirNew;
            bool ret_new = FindPathImplDStarLite(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
                                &firstDirNew, firstNodePos);

            RoadPathDirection firstDirOld;
            bool ret_old = FindPathImpl(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
                                &firstDirOld, firstNodePos);
            
            RTTR_Assert(ret_new == ret_old);
            RTTR_Assert(firstDirNew == firstDirOld);

            *firstDir = firstDirOld;
            return ret_old;
        }
    } else
    {
        if(forbidden) {
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::And<SegmentConstraints::AvoidSegment,
                                                        SegmentConstraints::AvoidRoadType<RoadType::Water>>(forbidden),
                                length, firstDir, firstNodePos);
        } else {
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::AvoidRoadType<RoadType::Water>(), length, firstDir, firstNodePos);
        }
    }
#endif
}

bool RoadPathFinder::PathExists(const noRoadNode& start, const noRoadNode& goal, const bool allowWaterRoads,
                                const unsigned max, const RoadSegment* const forbidden)
{
    if(allowWaterRoads)
    {
        if(forbidden)
            return FindPathImpl(start, goal, max, AdditonalCosts::None(), SegmentConstraints::AvoidSegment(forbidden));
        else
            return FindPathImpl(start, goal, max, AdditonalCosts::None(), SegmentConstraints::None());
    } else
    {
        if(forbidden)
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::And<SegmentConstraints::AvoidSegment,
                                                        SegmentConstraints::AvoidRoadType<RoadType::Water>>(forbidden));
        else
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::AvoidRoadType<RoadType::Water>());
    }
}
