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
#include "AsciiMap.h"
#include "gameData/GameConsts.h"
#include "s25util/Log.h"

/// Comparison operator for road nodes that returns true if lhs > rhs (descending order)
struct RoadNodeComperatorGreater
{
    bool operator()(const noRoadNode* const lhs, const noRoadNode* const rhs) const
    {
        if(lhs->estimate == rhs->estimate)
        {
            // Use a tie-breaker to get strict ordering
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

/// Path finding on roads using A* O(n lg n)
/// \tparam T_AdditionalCosts Cost for each road segment but the one to the goal building
/// \tparam T_SegmentConstraints Predicate whether a road is allowed
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

    // If the goal is a flag (unlikely) we have no goal building
    // TODO(Replay): Change RoadPathFinder::FindPath to target flag instead of building for wares
    const noRoadNode* goalBld = (goal.GetGOT() == GO_Type::Flag) ? nullptr : &goal;

    // Use a counter for the visited-states so we don't have to reset them on every invocation
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

            // Backtrack to get the last node that is not the start node (has a prev node)
            // --> Next node from start on path
            if(firstDir || firstNodePos)
            {
                const noRoadNode* firstNode = &best;
                while(firstNode->prev != &start)
                    firstNode = firstNode->prev;

                if(firstDir)
                    *firstDir = firstNode->dir_;

                if(firstNodePos)
                    *firstNodePos = firstNode->GetPos();
            }

            // Done, path found
            return true;
        }

        const helpers::EnumArray<RoadSegment*, Direction> routes = best.getRoutes();
        const noRoadNode* prevNode = best.prev;

        // Check paths in all directions
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

            // Check if route is forbidden
            if(!isSegmentAllowed(*route))
                continue;

            const unsigned cost = best.cost + route->GetLength() + (neighbour != goalBld ? addCosts(best, dir) : 0);

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

    // Queue is empty without reaching the goal --> No allowed, existing path
    return false;
}

bool RoadPathFinder::FindPath(const noRoadNode& start, const noRoadNode& goal, const bool wareMode, const unsigned max,
                              const RoadSegment* const forbidden, unsigned* const length,
                              RoadPathDirection* const firstDir, MapPoint* const firstNodePos)
{
    RTTR_Assert_Msg(length || firstDir || firstNodePos, "Use PathExists instead!");

    if(wareMode)
    {
        // TODO(Replay): Change to target flag instead of its attached building
        if(forbidden)
            return FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::AvoidSegment(forbidden), length, firstDir, firstNodePos);
        else {
            // return FindPathImpl(start, goal, max, AdditonalCosts::Carrier(), SegmentConstraints::None(), length,
            //                     firstDir, firstNodePos);
            return FindPathForWare(start, goal, max, length, firstDir, firstNodePos);
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

struct DStarContext
{
    const noRoadNode* goal;
    const MapBase& world;
    dstar::OpenList& openlist;

    dstar::Key CalculateKey(const noRoadNode* node)
    {
        dstar::Key ret;
        const auto& node_values = node->dstar.Get(goal);
        ret.k2 = std::min(node_values.g, node_values.rhs);
        ret.k1 = std::numeric_limits<unsigned>::max();

        if (ret.k2 == std::numeric_limits<unsigned>::max())
            return ret;

        if (ret.k2 > std::numeric_limits<unsigned>::max() - openlist.km) {
            ret.k1 = std::numeric_limits<unsigned>::max();
            return ret;
        }

        ret.k1 = ret.k2 + openlist.km;
        return ret;
    }

    dstar::Key CalculateKey(const dstar::QueueNode& node)
    {
        return CalculateKey(node.node);
    }

    dstar::NodeState& GetState(const noRoadNode* node)
    {
        return node->dstar.Get(goal);
    }

    dstar::NodeState& GetState(const dstar::QueueNode& node)
    {
        return node.node->dstar.Get(goal);
    }

    void UpdateVertex(const noRoadNode* node)
    {
        auto& node_state = GetState(node);
    
        // if u ≠ s_goal then
        if (&node != &goal) {
            // rhs(u) ← min_{s' ∈ Succ(u)} (c(u, s') + g(s'))
            unsigned best_cost = std::numeric_limits<unsigned>::max();
            for (const auto dir : helpers::EnumRange<Direction>{}) {
                const auto* route = node->getRoutes()[dir];
                if (!route)
                    continue;

                const noRoadNode* n = route->GetF1();
                if (n == node)
                    n = route->GetF2();

                const unsigned g = n->dstar.Get(goal).g;
                if (g == std::numeric_limits<unsigned>::max())
                    continue;

                unsigned cost = g;

                // we ignore paths to non-harbor buildings
                if (n->GetGOT() != GO_Type::Flag && n->GetGOT() != GO_Type::NobHarborbuilding)
                    cost += 0;
                else
                    cost = node->GetPunishmentPoints(dir) + g;
                if (cost < best_cost)
                    best_cost = cost;
            }
            node_state.rhs = best_cost;
        }

        // if u ∈ U then U.remove(u)
        openlist.Remove(node); //@todo: In case we have to push later anyway, we can just update the key

        // if g(u) ≠ rhs(u) then U.insert(u, CalculateKey(u))
        if (node_state.g != node_state.rhs)
            openlist.Push(dstar::QueueNode{const_cast<noRoadNode*>(node), CalculateKey(node)});
            
    }

    void UpdateVertex(const dstar::QueueNode& node)
    {
        UpdateVertex(node.node);
    }
};

#define DRAW_DSTAR_STEP \
    { \
        AsciiMap ascii(gwb_, goal.GetPos(), 6, 3, AsciiMap::Border::Locations); \
        ascii.drawPlayer(0); \
        ascii.drawDStar(&goal); \
        ascii.write(); \
    }

bool RoadPathFinder::FindPathForWare(
    const noRoadNode& start,
    const noRoadNode& goal, 
    unsigned max,
    unsigned* length, 
    RoadPathDirection* firstDir, 
    MapPoint* firstNodePos)
{
    std::cout << "Starting D*lite from (" << start.GetX() << "," << start.GetY() << ") to ("
              << goal.GetX() << "," << goal.GetY() << ")\n";

    if (&start == &goal) {
        if (length) *length = 0;
        return true;
    }

    // https://idm-lab.org/bib/abstracts/papers/aaai02b.pdf

    // Check if we ever searched for this goal
    if (!dstarU.Exists(&goal)) {
        // Initialize U and root node
        dstar::QueueNode rootNode{const_cast<noRoadNode*>(&goal), dstar::Key{gwb_.CalcDistance(goal.GetPos(), start.GetPos()), 0}};
        dstarU.Get(&goal).Push(rootNode);
        goal.dstar.Get(&goal).rhs = 0;

        DRAW_DSTAR_STEP
    }

    // Check for dirty nodes and update them
    auto& U = dstarU.Get(&goal);
    DStarContext context{ &goal, gwb_, U };

    if (!U.dirty_nodes.empty()) {
        U.km++;
        for (MapPoint pt : U.dirty_nodes) {
            auto* dirty_node = gwb_.GetSpecObj<noRoadNode>(pt);
            if (dirty_node)
                context.UpdateVertex(dirty_node);
        }
        U.dirty_nodes.clear();

        DRAW_DSTAR_STEP
    }

    // print U
    std::cout << "U contents:\n";
    for (const auto& qnode : U.queue) {
        if (qnode.node) {
            std::cout << "  Node (" << qnode.node->GetX() << "," << qnode.node->GetY() << ")"
                        << " key=(" << qnode.key.k1 << "," << qnode.key.k2 << ")"
                        << " g=" << qnode.node->dstar.Get(&goal).g
                        << " rhs=" << qnode.node->dstar.Get(&goal).rhs
                        << "\n";
        }
    }

    auto start_vals = start.dstar.Get(&goal);
    auto start_key = context.CalculateKey(&start);
    while (!U.queue.empty() && (U.Top().key < start_key || start_vals.rhs != start_vals.g))
    {
        const auto& u = U.Top();
        U.Remove(u);
        auto k_old = u.key;

        std::cout << "Visiting node " << u.node->GetX() << "," << u.node->GetY() << " with key (" << k_old.k1 << "," << k_old.k2 << ")\n";

        RTTR_Assert(u.node);

        auto k_new = context.CalculateKey(u);
        if (k_old < k_new) {
            U.Push(dstar::QueueNode{u.node, k_new});
        } else {
            auto& u_vals = context.GetState(u);
            if (u_vals.g > u_vals.rhs) {
                u_vals.g = u_vals.rhs;
            } else {
                u_vals.g = std::numeric_limits<unsigned>::max();
                context.UpdateVertex(u);
            }
            for (const auto dir : helpers::EnumRange<Direction>{}) {
                const auto* route = u.node->getRoutes()[dir];
                if (!route)
                    continue;

                const noRoadNode* n = route->GetF1();
                if (n == u.node)
                    n = route->GetF2();

                context.UpdateVertex(n); // @todo: skip buildings?
            }
        }
        
        DRAW_DSTAR_STEP

        start_vals = start.dstar.Get(&goal);
        start_key = context.CalculateKey(&start);

        // print U
        std::cout << "U contents:\n";
        for (const auto& qnode : U.queue) {
            if (qnode.node) {
                std::cout << "  Node (" << qnode.node->GetX() << "," << qnode.node->GetY() << ")"
                          << " key=(" << qnode.key.k1 << "," << qnode.key.k2 << ")"
                          << " g=" << qnode.node->dstar.Get(&goal).g
                          << " rhs=" << qnode.node->dstar.Get(&goal).rhs
                          << "\n";
            }
        }
    }

    start_vals = start.dstar.Get(&goal);
    if (start_vals.g > max || start_vals.g == std::numeric_limits<unsigned>::max())
        return false; // no path

    const noRoadNode* best = nullptr;
    unsigned best_cost = std::numeric_limits<unsigned>::max();
    Direction best_dir;

    for (const auto dir : helpers::EnumRange<Direction>{}) {
        const auto* route = start.getRoutes()[dir];
        if (!route)
            continue;

        const noRoadNode* n = route->GetF1();
        if (n == &start)
            n = route->GetF2();

        const auto& neighbour_vals = n->dstar.Get(&goal);
        if (neighbour_vals.g == std::numeric_limits<unsigned>::max())
            continue;
        unsigned cost = 0;
        if (goal.GetGOT() == GO_Type::Flag || goal.GetGOT() == GO_Type::NobHarborbuilding)
            cost = neighbour_vals.g + start.GetPunishmentPoints(dir);
        
        if (cost < best_cost) {
            best = n;
            best_cost = cost;
            best_dir = dir;
        }
    }

    //RTTR_Assert(best_cost == start_vals.g); <- fails sometimes here, why?
    if (length)
        *length = best_cost;
    if (firstDir)
        *firstDir = toRoadPathDirection(best_dir);
    if (firstNodePos && best)
        *firstNodePos = best->GetPos();
    return true;
}

bool RoadPathFinder::PathExists(const noRoadNode& start, const noRoadNode& goal, const bool allowWaterRoads,
                                const unsigned max, const RoadSegment* const forbidden)
{
    if(allowWaterRoads)
    {
        // TODO(Replay): Change to target flag instead of its attached building.
        // Likely combine with RoadPathFinder::FindPath
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

void RoadPathFinder::MarkEdgeDirty(const noRoadNode* goal, const noRoadNode* node)
{
    dstarU.Get(goal).AddDirty(node->GetPos());
}

void RoadPathFinder::OnNodeDestroyed(const noRoadNode* node, const GameWorldBase* world)
{
    if (dstarU.Exists(node))
    {
        // Remove this goal from all other nodes
        RTTR_FOREACH_PT(MapPoint, world->GetSize())
        {
            auto* const other_node = world->GetSpecObj<noRoadNode>(pt);
            if(other_node)
                other_node->dstar.Remove(node);
        }

        dstarU.Remove(node);
    }
}
