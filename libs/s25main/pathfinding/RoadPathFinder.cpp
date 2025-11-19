// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RoadPathFinder.h"
#include "EventManager.h"
#include "RttrForeachPt.h"
#include "buildings/nobHarborBuilding.h"
#include "pathfinding/DStarTypes.h"
#include "pathfinding/OpenListPrioQueue.h"
#include "pathfinding/OpenListVector.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noRoadNode.h"
#include "AsciiMap.h"
#include "gameData/GameConsts.h"
#include "s25util/Log.h"

//#define DEBUG_OUT_DSTAR

#ifdef DEBUG_OUT_DSTAR
#include <fstream>
#include <sstream>

#define DRAW_DSTAR_STEP \
    { \
        AsciiMap ascii(gwb_, goal.GetPos(), 10, 3, AsciiMap::Border::Locations); \
        ascii.drawDStar(goal.GetPos()); \
        ascii.write(out_buffer); \
        out_buffer << "U contents:\n"; \
        for (const auto& qnode : U.queue) { \
            out_buffer << "  Node (" << qnode.nodePos.x << "," \
                        << qnode.nodePos.y << ")" \
                        << " key=" << qnode.key << "\n"; \
        } \
    }
#define DRAW_DSTAR_STEP_STDOUT \
    { \
        AsciiMap ascii(gwb_, goal.GetPos(), 10, 3, AsciiMap::Border::Locations); \
        ascii.drawDStar(goal.GetPos()); \
        ascii.write(); \
        std::cout << "U contents:\n"; \
        for (const auto& qnode : U.queue) { \
            std::cout << "  Node (" << qnode.nodePos.x << "," \
                        << qnode.nodePos.y << ")" \
                        << " key=" << qnode.key << "\n"; \
        } \
    }
#else
#define DRAW_DSTAR_STEP
#define DRAW_DSTAR_STEP_STDOUT
#endif

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

#define USE_DSTAR

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
#ifndef NDEBUG
            static unsigned dbg = 0;
            dbg++;
            unsigned length1;
            bool success = FindPathForWare(start, goal, max, &length1, firstDir, firstNodePos);

            unsigned length2;
            RoadPathDirection firstDir2;
            MapPoint firstNodePos2;
            bool success2 = FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::None(), &length2, &firstDir2, &firstNodePos2);
            if (success2 != success || (success && (length1 != length2))) {
                AsciiMap ascii(gwb_, goal.GetPos(), 20, 3, AsciiMap::Border::Locations);
                for (unsigned i = 0; i < gwb_.GetNumPlayers(); ++i)
                    ascii.drawPlayer(i);
                ascii.drawDStar(goal.GetPos());
                ascii.write();
                unsigned length3 = 0;
                FindPathForWare(start, goal, max, &length3, firstDir, firstNodePos);
                FindPathImpl(start, goal, max, AdditonalCosts::Carrier(),
                                SegmentConstraints::None(), &length2, &firstDir2, &firstNodePos2);
                RTTR_Assert(false);
            }

            if (length) *length = length1;
            return success;
#else
            return FindPathForWare(start, goal, max, length, firstDir, firstNodePos);
#endif
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
    const MapPoint goal;
    const GameWorldBase& world;
    dstar::OpenList& openlist;

    unsigned CalculateKey(const noRoadNode* node)
    {
        const auto& node_values = node->dstar.Get(goal);
        return std::min(node_values.g, node_values.rhs)/* + world.CalcDistance(node->GetPos(), goal)*/;
    }

    dstar::NodeState& GetState(const noRoadNode* node)
    {
        return node->dstar.Get(goal);
    }

    void UpdateVertex(const MapPoint nodePos)
    {
        auto* const node = world.GetSpecObj<noRoadNode>(nodePos);

        // node might not exist anymore
        if (!node)
            return;

        auto& node_state = GetState(node);
    
        // if u ≠ s_goal then
        if (nodePos != goal) {
            // rhs(u) ← min_{s' ∈ Succ(u)} (c(u, s') + g(s'))
            unsigned best_cost = std::numeric_limits<unsigned>::max();
            for (const auto dir : helpers::EnumRange<Direction>{}) {
                const auto* route = node->getRoutes()[dir];
                if (!route)
                    continue;

                const noRoadNode* n = route->GetF1();
                if (n == node)
                    n = route->GetF2();

                if (dir == Direction::NorthWest && n->GetPos() != goal) {
                    // Flags and harbors are allowed
                    const GO_Type got = n->GetGOT();
                    if(got != GO_Type::Flag && got != GO_Type::NobHarborbuilding)
                        continue;
                }

                const unsigned g = n->dstar.Get(goal).g;
                if (g == std::numeric_limits<unsigned>::max())
                    continue;

                unsigned cost = g + route->GetLength();

                // we ignore additional path costs to non-harbor buildings
                if (n->GetPos() != goal && (n->GetGOT() == GO_Type::Flag || n->GetGOT() == GO_Type::NobHarborbuilding))
                    cost += node->GetPunishmentPoints(dir);

                if (cost < best_cost)
                    best_cost = cost;
            }

            // @todo if harbor: for all other harbor connections
            node_state.rhs = best_cost;
        }

        // if u ∈ U then U.remove(u)
        openlist.Remove(node->GetPos()); // @todo: In case we have to push later anyway, we can just update the key

        // if g(u) ≠ rhs(u) then U.insert(u, CalculateKey(u))
        if (node_state.g != node_state.rhs)
            openlist.Push(dstar::QueueNode{node->GetPos(), CalculateKey(node)});
    }

    void UpdateVertex(const dstar::QueueNode& node)
    {
        UpdateVertex(node.nodePos);
    }
};

bool RoadPathFinder::FindPathForWare(
    const noRoadNode& start,
    const noRoadNode& goal, 
    unsigned max,
    unsigned* length, 
    RoadPathDirection* firstDir, 
    MapPoint* firstNodePos)
{
    // This function implements D*lite pathfinding as described in
    // https://idm-lab.org/bib/abstracts/papers/aaai02b.pdf
    //
    // Abberrations from the original algorithm:
    // - We use multiple agents (calls to FindPathForWare) with the same goal.
    //   Therefore we maintain a global open list (dstarU) per goal node.
    //   For the same reason, we cannot use the km value as described in the paper,
    //   instead we increment it whenever we have dirty nodes to process.

    RTTR_Assert(goal.GetType() == NodalObjectType::Building || goal.GetType() == NodalObjectType::Buildingsite);
    const auto& goalBuilding = dynamic_cast<const noBaseBuilding&>(goal);

#ifdef DEBUG_OUT_DSTAR
    std::ostringstream out_buffer;
    static unsigned file_idx = 0;
    std::string filename = "dstar_debug." + std::to_string(file_idx++) + "." + std::to_string(start.GetPos().x) + "_" + std::to_string(start.GetPos().y) + "." + std::to_string(goal.GetPos().x) + "_" + std::to_string(goal.GetPos().y) + ".txt";
    out_buffer << "Starting D*lite from (" << start.GetX() << "," << start.GetY() << ") to (" << goal.GetX() << "," << goal.GetY() << ")\n";
    std::cout << "Starting D*lite from (" << start.GetX() << "," << start.GetY() << ") to (" << goal.GetX() << "," << goal.GetY() << ")\n";
#endif

    // Catch most simple case where start and goal are the same
    if (&start == &goal) {
        if (length) *length = 0;
        if(firstDir) *firstDir = RoadPathDirection::None;
        if(firstNodePos) *firstNodePos = start.GetPos();
#ifdef DEBUG_OUT_DSTAR
        std::ofstream out(filename, std::ios::trunc);
        out << out_buffer.str();
        out.close();
#endif
        return true;
    }

    // Check if we ever searched for this goal and initialize U if not.
    if (!goalBuilding.dstarU) {
        goalBuilding.dstarU = std::make_shared<dstar::OpenList>();
        dstar::QueueNode rootNode{goal.GetPos(), 0};
        goalBuilding.dstarU->Push(rootNode);
        goal.dstar.Get(goal.GetPos()).rhs = 0;
    }

    auto& U = *goalBuilding.dstarU;
    DStarContext context{ goal.GetPos(), gwb_, U };
    
    DRAW_DSTAR_STEP

    auto start_vals = start.dstar.Get(goal.GetPos());
    auto start_key = context.CalculateKey(&start);
    while (!U.queue.empty() && (U.Top().key < start_key || start_vals.rhs != start_vals.g))
    {
        const auto& u = U.Top();
        U.Remove(u.nodePos);
        auto k_old = u.key;

#ifdef DEBUG_OUT_DSTAR
        out_buffer << "Visiting node " << u.nodePos.x << "," << u.nodePos.y << "\n";
#endif

        auto* const u_node = gwb_.GetSpecObj<noRoadNode>(u.nodePos);
        if (!u_node)
            continue; // node got deleted

        auto k_new = context.CalculateKey(u_node);
        if (k_old < k_new) {
            U.Push(dstar::QueueNode{u.nodePos, k_new});
        } else {
            auto& u_vals = context.GetState(u_node);
            if (u_vals.g > u_vals.rhs) {
                u_vals.g = u_vals.rhs;
            } else {
                u_vals.g = std::numeric_limits<unsigned>::max();
                RTTR_Assert(u.nodePos != goal.GetPos());
                context.UpdateVertex(u);
            }
            for (const auto dir : helpers::EnumRange<Direction>{}) {
                const auto* route = u_node->getRoutes()[dir];
                if (!route)
                    continue;

                const noRoadNode* n = route->GetF1();
                if (n == u_node)
                    n = route->GetF2();

                context.UpdateVertex(n->GetPos());
            }
        }
        
        start_vals = start.dstar.Get(goal.GetPos());
        start_key = context.CalculateKey(&start);

        DRAW_DSTAR_STEP
    }

    start_vals = start.dstar.Get(goal.GetPos());
    if (start_vals.g > max || start_vals.g == std::numeric_limits<unsigned>::max()) {
        if (start_vals.g == std::numeric_limits<unsigned>::max() && start_vals.rhs == std::numeric_limits<unsigned>::max()) {
            // do not store rhs and g values for unreachable nodes
            RTTR_Assert(start.GetPos() != goal.GetPos());
            start.dstar.Remove(goal.GetPos());
        }

#ifdef DEBUG_OUT_DSTAR
        out_buffer << "No path found, start g=" << start_vals.g << " rhs=" << start_vals.rhs << "\n";
        std::ofstream out(filename, std::ios::trunc);
        out << out_buffer.str();
        out.close();
#endif
        return false; // no path
    }

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

        const auto& neighbour_vals = n->dstar.Get(goal.GetPos());
        if (neighbour_vals.g == std::numeric_limits<unsigned>::max())
            continue;
        unsigned cost = neighbour_vals.g + route->GetLength();
        if (n != &goal && (n->GetGOT() == GO_Type::Flag || n->GetGOT() == GO_Type::NobHarborbuilding))
            cost += start.GetPunishmentPoints(dir);
        
        if (cost < best_cost) {
            best = n;
            best_cost = cost;
            best_dir = dir;
        }
    }

    // @todo if harbor: for all other harbor connections

    //RTTR_Assert(best_cost == start_vals.g);
    if (best_cost != start_vals.g)
    {
        DRAW_DSTAR_STEP_STDOUT
    }

    if (length)
        *length = best_cost;
    if (firstDir)
        *firstDir = toRoadPathDirection(best_dir);
    if (firstNodePos && best)
        *firstNodePos = best->GetPos();

#ifdef DEBUG_OUT_DSTAR
    out_buffer << "Path found with length " << best_cost << ", first dir " << unsigned(toRoadPathDirection(best_dir)) << "\n";
    std::ofstream out(filename, std::ios::trunc);
    out << out_buffer.str();
    out.close();
#endif

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

void RoadPathFinder::MarkNodeDirty(const noRoadNode* node)
{
    RTTR_Assert(node);

    // If this node is not part of any pathfinding, we can skip it.
    // if (node->dstar.Empty())
    //     return;

    // calculate costs of to all adjacent nodes
    std::vector<std::tuple<const noRoadNode*, unsigned, unsigned>> neighbours;
    for (const auto dir : helpers::EnumRange<Direction>{}) {
        const auto* route = node->getRoutes()[dir];
        if (!route)
            continue;

        const noRoadNode* neighbour = route->GetF1();
        if (neighbour == node)
            neighbour = route->GetF2();

        unsigned edge_cost = 0;
        if (neighbour->GetGOT() == GO_Type::Flag || neighbour->GetGOT() == GO_Type::NobHarborbuilding)
            edge_cost = node->GetPunishmentPoints(dir);

        neighbours.push_back({neighbour, edge_cost, route->GetLength()});
    }

    // loop over all goals that use this node
    for (auto& [goal_pos, state] : node->dstar.map) {
        noBaseBuilding* goal_bld = gwb_.GetSpecObj<noBaseBuilding>(goal_pos);
        if (!goal_bld)
            continue;

        if (goal_pos != node->GetPos()) {
            unsigned best_cost = std::numeric_limits<unsigned>::max();
            for (const auto& [neighbour, edge_cost, route_len] : neighbours) {
                // edges that lead to a building which is not the goal are ignored
                if (neighbour->GetPos() != goal_pos && neighbour->GetGOT() != GO_Type::Flag && neighbour->GetGOT() != GO_Type::NobHarborbuilding)
                    continue;

                unsigned cost = neighbour->dstar.Get(goal_pos).g;
                if (cost == std::numeric_limits<unsigned>::max())
                    continue;

                cost += route_len;

                // we ignore additional path costs to non-harbor buildings
                if (neighbour->GetPos() != goal_pos && (neighbour->GetGOT() != GO_Type::Flag || neighbour->GetGOT() != GO_Type::NobHarborbuilding))
                    cost += edge_cost;

                best_cost = std::min(best_cost, cost);
            }
            // @todo if harbor: for all other harbor connections
            
            state.rhs = best_cost;
        }

        auto U = goal_bld->dstarU;
        U->Remove(node->GetPos());
        if (state.g != state.rhs)
            U->Push(dstar::QueueNode{node->GetPos(), std::min(state.g, state.rhs)});
    }
}

void RoadPathFinder::OnNodeDestroyed(const noRoadNode* node, const GameWorldBase* world)
{
    const noBaseBuilding* goalBuilding = dynamic_cast<const noBaseBuilding*>(node);

    if (goalBuilding && goalBuilding->dstarU)
        goalBuilding->dstarU.reset();

    RTTR_FOREACH_PT(MapPoint, world->GetSize())
    {
        auto* const other_node = world->GetSpecObj<noRoadNode>(pt);
        if (other_node)
            other_node->dstar.Remove(node->GetPos());
    }
}
