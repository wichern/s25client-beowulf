// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "world/MapBase.h"
#include "helpers/EnumRange.h"
#include "gameTypes/Direction.h"
#include "gameTypes/BuildingType.h"
#include "gameData/BuildingProperties.h"

#include <vector>
#include <queue>
#include <map>

namespace beowulf {

struct MapPointComp
{
    bool operator()(const MapPoint& lhs, const MapPoint& rhs) const {
        return (lhs.y < rhs.y) || ((lhs.y == rhs.y) && (lhs.x < rhs.x));
    }
};

inline Direction OppositeDirection(Direction dir) {
    return dir + 3;
}

// generic DFS search
template<typename Condition, typename Action>
void FloodFill(
        const MapBase& world,
        const MapPoint& start,
        Condition condition,
        Action action)
{
    std::vector<bool> visited;
    visited.resize(world.GetSize().x * world.GetSize().y);

    std::vector<MapPoint> pts;
    pts.push_back(start);
    visited[world.GetIdx(start)] = true;

    while (!pts.empty()) {
        MapPoint cur = pts.back();
        pts.pop_back();

        action(cur);

        for (const auto dir : helpers::EnumRange<Direction>{}) {
            MapPoint next = world.GetNeighbour(cur, dir);
            unsigned next_idx = world.GetIdx(next);
            if (!visited[next_idx] && condition(cur, dir)) {
                pts.push_back(next);
                visited[next_idx] = true;
            }
        }
    }
}

// generic A* search
//
// @param start     Start point
// @param world     World
// @param route     found route (optional)
// @param condition edge condition
// @param end       end condition  (e.g. [&](const MapPoint& pt) { return pt == dest; })
// @param heuristic A* heuristic (e.g. [&](const MapPoint& pt) { return gwb.CalcDistance(pt, dest); })
// @param cost      Cost function (e.g. [&](const MapPoint& pt, Direction dir) { return 1; })
template<typename Condition, typename End, typename Heuristic, typename Cost>
bool FindPath(
        const MapPoint& start,
        const MapBase& world,
        std::vector<Direction>* route,
        Condition condition,
        End end,
        Heuristic heuristic,
        Cost cost)
{
    typedef unsigned distance_t;
    typedef unsigned cost_t;
    typedef std::tuple<MapPoint, cost_t, distance_t> front_pos_t;

    struct PosCompare
    {
        bool operator()(const front_pos_t& l, const front_pos_t& r) const
        { return std::get<1>(l) > std::get<1>(r); }
    };
    std::priority_queue<front_pos_t, std::vector<front_pos_t>, PosCompare> frontier;
    std::map<MapPoint, Direction, MapPointComp> came_from; // @performance: use array?
    std::map<MapPoint, cost_t, MapPointComp> cost_so_far;
    cost_so_far[start] = 0;
    MapPoint dest;

    frontier.push({ start, 0, 1 });

    while (!frontier.empty()) {
        MapPoint cur;
        distance_t len;
        std::tie(cur, std::ignore, len) = frontier.top();
        frontier.pop();

        if (cur != start && end(cur)) {
            dest = cur;
            break;
        }

        for (const auto dir : helpers::EnumRange<Direction>{}) {
            if (!condition(cur, dir, len))
                continue;

            cost_t new_cost = cost_so_far[cur] + cost(cur, dir);
            MapPoint next = world.GetNeighbour(cur, dir);
            if (cost_so_far.find(next) == cost_so_far.end() ||
                    new_cost < cost_so_far[next])
            {
                cost_so_far[next] = new_cost;
                frontier.push({ next, new_cost + heuristic(next), len + 1 });
                came_from[next] = dir;
            }
        }
    }

    if (!dest.isValid())
        return false;

    if (route) {
        std::vector<Direction> reverse;
        MapPoint cur = dest;
        while (cur != start) {
            Direction dir = came_from[cur];
            reverse.push_back(dir);
            cur = world.GetNeighbour(cur, OppositeDirection(dir));
        }

        route->clear();
        route->reserve(reverse.size());
        for (int i = reverse.size() - 1; i >= 0; --i)
            route->push_back(reverse[i]);
    }

    return true;
}

inline unsigned BuildingTypeWithoutUnused2I(BuildingType bld) {
    RTTR_Assert(BuildingProperties::IsValid(bld));
    unsigned ret = 0u;
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (bld == i)
            return ret;
        if (BuildingProperties::IsValid(i))
            ret++;
    }
    RTTR_Assert(false);
    return ret;
}

inline BuildingType BuildingTypeWithoutUnused(unsigned bld) {
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (!BuildingProperties::IsValid(i))
            continue;
        if (0 == bld)
            return i;
        bld--;
    }
    RTTR_Assert(false);
    return BuildingType::Nothing2;
}

} // namespace beowulf