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

OpenListVector<const noRoadNode*, GetKeyFromPtr> todoDLite;

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

static const unsigned g_dstarNodeCountLimit = 10; // how many nodes can a vector contain?
static const unsigned g_dstarNodeAgeLimit = 1000; // how old can a node be?

// Get the node or create a new one.
// If number of nodes exceeds threshold, we remove the lru node.
static noRoadNode::DStarNode& GetNode(const noRoadNode& node, const MapPoint& dest, unsigned gf)
{
    auto oldest_it = node.dstar_.begin();

    for(auto it = node.dstar_.begin(); it != node.dstar_.end(); ++it)
    {
        if(it->dest == dest)
        {
            it->visit_gf = gf;
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
    node.dstar_.push_back({dest, g, rhs, gf});
    return node.dstar_.back();
}

} // namespace dstarlite

template<class T_AdditionalCosts, class T_SegmentConstraints>
bool RoadPathFinder::FindPathImplDStarLite(const noRoadNode& start, const noRoadNode& goal, const unsigned max,
                                           const T_AdditionalCosts addCosts,
                                           const T_SegmentConstraints isSegmentAllowed, unsigned* const length,
                                           RoadPathDirection* const firstDir, MapPoint* const firstNodePos)
{
#if 0

[ ] 1. ComputeShortestPath() ausprobieren, um zu sehen, ob es den selben Pfad wie die alte Implementierung liefert.

[ ] 2. Wenn start->GetNode(dest) ein Ergebnis liefert, dann können wir iterativ weitermachen.
        (wähle Nachbarknoten mit niedrigstem c(start,neighbour')+neigbour->g) )

Kostenänderung auf noRoadNode:
    Bei call von AddWare(), DestroyRoad(), DestroyAllRoads(), SetRoute()

#endif

    // one all g,rhs values are calculated, we can find the shortest path by checking
    // every neighbour node (n) that has the smallest (edgeCost(this, n) + n->g) until we reach dest.

    /*
    The pseudocode uses the following functions to manage the priority queue:
    - U.Top() returns a vertex with the smallest priority of all vertices in priority queue U.
    - U.TopKey() returns the smallest priority of all vertices in priority queue U. (If U is empty, then U.TopKey()
    returns W X ZY[X;\ .)
    - U.Pop() deletes the vertex with the smallest priority in priority queue U and returns the vertex.
    - U.Insert(s, k) inserts vertex s into priority queue U with priority k.
    - U.Update(s, k) changes the priority of vertex s in priority queue U to k. (It does nothing if the current priority
    of vertex s already equals k)
    - U.Remove(s) removes vertex s from priority queue U.
    */
    // k1, k2 ... Key value calcuated from g and rhs
    // k_m ... ?

    // Create queue
    //  OpenListVector<std::pair<const noRoadNode*, const noRoadNode::DStarNode*>, GetKeyFromSecond>

    // procedure noRoadNode.dstarNode[dest].CalculateKey(unsigned h)
    //      Calculates the key for a node. The key is used in the prio-queue.
    //  k2 = std::min(this.g, this.rhs)
    //  k1 = k2 + h + k_m

    // procedure Initialize(noRoadNode* start, noRoadNode* dest)
    //      Initializes all nodes to infinity. We skip that part and lazyly add every unknown node with
    //      an initial value of inifinity (aka std::limits<unsigned>::max())
    //  todo.clear();
    //  k_m = 0;
    //  dest_node = dest.dstarNode[dest.GetPos()];
    //  dest_node->rhs = 0;
    //  dest_node->CalculateKey(gwb.CalcDistance(dest.GetPos(), start.GetPos());
    //  todo.push({dest, dest_node});

    // procedure UpdateVertex(noRoadNode* u, noRoadNode::DStarNode* u_node, noRoadNode* dest)
    //  bool u_in_queue = NodeInQueue(u, todo)
    //  if (u_node->g != u_node->rhs)  // locally inconsistent
    //      u_node->CalculateKey(gwb->CalcDistance(dest->GetPos(), u->GetPos()));
    //      if (!u_in_queue)
    //          todo.push({u, u_node})
    //  else {
    //      if (u_in_queue) {
    //          u_node.k.k1 = 0;  // remove element by decreasing key to smallest possible value
    //          u_node.k.k2 = 0;
    //          todo.pop()
    //      }
    // }

    // procedure ComputeShortestPath(start_node, dest)
    //  {u, u_node} = todo.pop();
    //  while (u_node.key > start_node.key || start_node.rhs > start_node.g) {
    //      k_old = u_node->k;
    //      u_node->CalculateKey(gwb->CalcDistance(dest->GetPos(), u->GetPos()));
    //      if (k_old < u_node->k)
    //          todo.push({u, u_node});
    //      else if (u_node->g > u_node->rhs) {
    //          u_node->g = u_node->rhs;
    //          // update rhs value of all neighbours that are not the goal node { 20 }
    //          UpdateVertex(u, u_node, dest);
    //      } else {
    //          g_old = u_node->g;
    //          u_node->g = std::numeric_limits<unsigned>::max();
    //          // for all neighbours and u as s, s_node:
    //              if (s_node->rhs == cost(s, u) + g_old)
    //                  if (s != dest) s_node->rhs = SmallestCostNeighbourOf(s);
    //              UpdateVertex(s, s_node, dest)
    //      }
    //
    //      {u, u_node} = todo.pop()
    //

    // Initialize()
    /* {01} */ todoDLite.clear();
    /* {02} */ unsigned k_m = 0;
    /* {03} is done by dstarlite::GetNode setting every new node to g = rhs = MAX */
    noRoadNode::DStarNode& dnode_goal = GetNode(goal, dest, currentGf);
    /* {04} */ dnode_goal.rhs = 0;
    /* {05} */ dnode_goal.k1 = gwb.CalcDistance(start.GetPos(), goal.GetPos(); dnode_goal.k2 = 0; U.push(&dnode_goal);

                                                // ComputeShortestPath()
                                                unsigned currentGf = gwb_.GetEvMgr().GetCurrentGF();
                                                noRoadNode::DStarNode& dnode_start = GetNode(start, dest, currentGf);
                                                /* {10} */ while(todoDLite.front().< dnode_start.CalculateKey)
                                                || dnode_start.rhs > dnode_start.g)
    {
        /* {11} */ auto& u = U.front();
        /* {12} */ dstarlite::Key k_old = u.second;
        /* {13} */ dstarlite::Key k_new = CalculateKey(u.first);
        /* {14} */ if(k_old < k_new)
        {
            /* {15} */ u.key = k_new;
            todoDLite.push(u);
            /* {16} */ else if(u.g > u.rhs)
            {
                /* {17} */ u.g = u.rhs;
                /* {18} */ // we do not remove but push in else case instead, because pop() already removed it
                /* {19} */ for(const RoadSegment* route : u.getRoutes())
                {
                    // Check the 2 flags, one is the current node, so we need the other
                    noRoadNode* s = route->GetF1();
                    if(s == &u)
                        s = route->GetF2();
                    unsigned cost = route->GetLength() + addCosts();
                    cost += addCosts(u, route);
                    /* {20} */ if(s != goal)
                    {
                        s.rhs = std::min(s.rhs, )
                    }
                }
            }
            else
            {
                todoDLite.push(u);
            }
        }
    }
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

        const helpers::EnumArray<RoadSegment*, Direction> routes = best.getRoutes();
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
        if(forbidden)
        {
            return FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::AvoidSegment(forbidden), length, firstDir, firstNodePos);
        } else
        {
            RoadPathDirection firstDirNew;
            bool ret_new = FindPathImplDStarLite(start, goal, max, AdditonalCosts::Carrier(),
                                                 SegmentConstraints::None(), length, &firstDirNew, firstNodePos);

            RoadPathDirection firstDirOld;
            bool ret_old = FindPathImpl(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
                                        &firstDirOld, firstNodePos);

            RTTR_Assert(ret_new == ret_old);
            RTTR_Assert(firstDirNew == firstDirOld);

            *firstDirOld = firstDirOld;
            return ret_old;
        }
    } else
    {
        if(forbidden)
        {
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::And<SegmentConstraints::AvoidSegment,
                                                        SegmentConstraints::AvoidRoadType<RoadType::Water>>(forbidden),
                                length, firstDir, firstNodePos);
        } else
        {
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
