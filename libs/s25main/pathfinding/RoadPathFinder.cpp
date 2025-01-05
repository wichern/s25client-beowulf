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
#include "AsciiMap.h"

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
    bool operator()(const std::shared_ptr<VirtualRoadSegment>&) const { return true; }
};

/// Disallows a specific road segment
struct AvoidSegment
{
    const RoadSegment* const forbiddenSeg_;
    AvoidSegment(const RoadSegment* const forbiddenSeg) : forbiddenSeg_(forbiddenSeg) {}

    bool operator()(const RoadSegment& segment) const { return forbiddenSeg_ != &segment; }
    bool operator()(const std::shared_ptr<VirtualRoadSegment>& /*segment*/) const{
        // @todo: check all elements in vroute
        return false;
    }
};

/// Disallows a specific road type
template<RoadType T_roadType>
struct AvoidRoadType
{
    bool operator()(const RoadSegment& segment) const { return segment.GetRoadType() != T_roadType; }
    bool operator()(const std::shared_ptr<VirtualRoadSegment>& segment) const { return segment->roadType != T_roadType; }
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

    bool operator()(const std::shared_ptr<VirtualRoadSegment>& segment) const
    {
        return Func1::operator()(segment) && Func2::operator()(segment);
    }
};
} // namespace SegmentConstraints

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
            // @todo: check on vroute as well
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

#include "PointOutput.h"
#include "gameTypes/GameTypesOutput.h"
#include <iostream>

/// Wegfinden ( A* ), O(v lg v) --> Wegfindung auf Stra�en
template<class T_AdditionalCosts, class T_SegmentConstraints>
bool RoadPathFinder::FindPathImplFast(const noRoadNode& start, const noRoadNode& goal, const unsigned max,
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
        const helpers::EnumArray<std::shared_ptr<VirtualRoadSegment>, Direction>& vroutes = best.getVRoutes();
        const noRoadNode* prevNode = best.prev;

        // Nachbarflagge bzw. Wege in allen 6 Richtungen verfolgen
        for(const auto dir : helpers::EnumRange<Direction>{})
        {
            const auto* route = routes[dir];
            if(!route)
                continue;

            auto* f1 = route->GetF1();
            auto* f2 = route->GetF2();

            // if we have a vroute as well, we take that instead
            auto vroute = vroutes[dir];
            if (vroute) {
                f1 = vroute->f1;
                f2 = vroute->f2;
            }

            // On a normal route,'this' is either 'f1' or 'f2' and the neighbour is always the other route
            // On a vroute, this is not true. We have to follow the vroute in the given direction to know which one is the neighbour
            const noRoadNode* neighbour = nullptr;

            if (vroute)
                neighbour = best.GetNeighbour(dir, vroute);
            else {
                if (&best == f1)
                    neighbour = f2;
                else
                    neighbour = f1;
            }

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
            // @todo: check on vroute as well
            unsigned cost = best.cost;
            if (vroute) {
                if (!isSegmentAllowed(vroute))
                    continue;
                cost += best.GetVRouteLengthAndCosts(dir, addCosts);
            } else {
                if(!isSegmentAllowed(*route))
                    continue;
                cost += route->GetLength();
                cost += addCosts(best, dir);
            }

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
                    std::cout << best.GetPos() << " to " << neighbour->GetPos() << ": " << cost << std::endl;
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
                std::cout << best.GetPos() << " to " << neighbour->GetPos() << ": " << cost << std::endl;

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

bool RoadPathFinder::FindPath(const noRoadNode& start, const noRoadNode& goal, const bool wareMode, const unsigned max,
                              const RoadSegment* const forbidden, unsigned* const length,
                              RoadPathDirection* const firstDir, MapPoint* const firstNodePos)
{
    RTTR_Assert(length || firstDir || firstNodePos); // If none of them is set use the \ref PathExist function!

    if(wareMode)
    {
        if(forbidden)
            return FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::AvoidSegment(forbidden), length, firstDir, firstNodePos);
        else {
            bool ret = FindPathImpl(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
                                firstDir, firstNodePos);

            unsigned length_fast = 0;
            RoadPathDirection firstDir_fast;
            MapPoint firstNodePos_fast;
            // {
            //     AsciiMap debug(gwb_, start.GetPos(), 10);
            //     for (unsigned playerId = 0; playerId < gwb_.GetNumPlayers(); ++playerId)
            //     {
            //         debug.drawPlayer(playerId);
            //         debug.drawVroutes(playerId);
            //     }
            //     debug.write();
            // }
            bool ret_fast = FindPathImplFast(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), &length_fast,
                                &firstDir_fast, &firstNodePos_fast);


            if (ret != ret_fast)
            {
                AsciiMap debug(gwb_, start.GetPos(), 10);
                for (unsigned playerId = 0; playerId < gwb_.GetNumPlayers(); ++playerId)
                {
                    debug.drawPlayer(playerId);
                    debug.drawVroutes(playerId);
                }
                debug.write();

                ret_fast = FindPathImplFast(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), &length_fast,
                                &firstDir_fast, &firstNodePos_fast);
                RTTR_Assert(ret == ret_fast);
            }

            if (ret && length) {
                RTTR_Assert(*length == length_fast);
            }
            if (ret && firstDir && *firstDir != firstDir_fast) {
                AsciiMap debug(gwb_, start.GetPos(), 10);
                for (unsigned playerId = 0; playerId < gwb_.GetNumPlayers(); ++playerId)
                {
                    debug.drawPlayer(playerId);
                    debug.drawVroutes(playerId);
                }
                debug.write();
                ret_fast = FindPathImplFast(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), &length_fast,
                                &firstDir_fast, &firstNodePos_fast);
                RTTR_Assert(*firstDir == firstDir_fast);
            }
            // if (ret && firstNodePos) {
            //     RTTR_Assert(*firstNodePos == firstNodePos_fast);
            // }

            return ret;
        }
    } else
    {
        if(forbidden)
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::And<SegmentConstraints::AvoidSegment,
                                                        SegmentConstraints::AvoidRoadType<RoadType::Water>>(forbidden),
                                length, firstDir, firstNodePos);
        else
            return FindPathImpl(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::AvoidRoadType<RoadType::Water>(), length, firstDir, firstNodePos);
    }
}

bool RoadPathFinder::FindPathFast(const noRoadNode& start, const noRoadNode& goal, const bool wareMode, const unsigned max,
                              const RoadSegment* const forbidden, unsigned* const length,
                              RoadPathDirection* const firstDir, MapPoint* const firstNodePos)
{
    RTTR_Assert(length || firstDir || firstNodePos); // If none of them is set use the \ref PathExist function!

    if(wareMode)
    {
        if(forbidden)
            return FindPathImplFast(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::AvoidSegment(forbidden), length, firstDir, firstNodePos);
        else
            return FindPathImplFast(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
                                firstDir, firstNodePos);
    } else
    {
        if(forbidden)
            return FindPathImplFast(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::And<SegmentConstraints::AvoidSegment,
                                                        SegmentConstraints::AvoidRoadType<RoadType::Water>>(forbidden),
                                length, firstDir, firstNodePos);
        else
            return FindPathImplFast(start, goal, max, AdditonalCosts::None(),
                                SegmentConstraints::AvoidRoadType<RoadType::Water>(), length, firstDir, firstNodePos);
    }
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
