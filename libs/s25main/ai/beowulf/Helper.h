// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef BEOWULF_HELPER_H_INCLUDED
#define BEOWULF_HELPER_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/MapBase.h"
#include "gameData/MilitaryConsts.h"

#include <utility>
#include <vector>
#include <map>
#include <queue>

namespace beowulf {

class ProductionGroups;
class World;

struct MapPointComp
{
    bool operator()(const MapPoint& lhs, const MapPoint& rhs) const {
        return (lhs.y < rhs.y) || ((lhs.y == rhs.y) && (lhs.x < rhs.x));
    }
};

inline Direction OppositeDirection(Direction dir) {
    if (dir.toUInt() < 3)
        return Direction(dir.toUInt() + 3);
    else
        return Direction(dir.toUInt() - 3);
}

inline unsigned GetMilitaryRadius(BuildingType type) {
    switch (type) {
    case BLD_BARRACKS: return MILITARY_RADIUS[0];
    case BLD_GUARDHOUSE: return MILITARY_RADIUS[1];
    case BLD_WATCHTOWER: return MILITARY_RADIUS[2];
    case BLD_FORTRESS: return MILITARY_RADIUS[3];
    case BLD_HARBORBUILDING: return HARBOR_RADIUS;
    case BLD_HEADQUARTERS: return HQ_RADIUS;
    default: return 0;
    }
}

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

        for (Direction dir : Direction()) {
            MapPoint next = world.GetNeighbour(cur, dir);
            unsigned next_idx = world.GetIdx(next);
            if (!visited[next_idx] && condition(cur, dir)) {
                pts.push_back(next);
                visited[next_idx] = true;
            }
        }
    }
}

/**
 * @brief Search route from 'start' to the first point that matches the 'end' condition.
 */
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
    typedef std::pair<MapPoint, distance_t> front_pos_t;

    struct PosCompare
    {
        bool operator()(const front_pos_t& l, const front_pos_t& r) const
        { return l.second > r.second; }
    };
    std::priority_queue<front_pos_t, std::vector<front_pos_t>, PosCompare> frontier;
    std::map<MapPoint, Direction, MapPointComp> came_from; // @performance: use array?
    std::map<MapPoint, distance_t, MapPointComp> cost_so_far;
    cost_so_far[start] = 0;
    MapPoint dest;

    frontier.push({ start, 0 });

    while (!frontier.empty()) {
        MapPoint cur = frontier.top().first;
        frontier.pop();

        if (cur != start && end(cur)) {
            dest = cur;
            break;
        }

        for (Direction dir : Direction()) {
            if (!condition(cur, dir))
                continue;

            distance_t new_cost = cost_so_far[cur] + cost(cur, dir);
            MapPoint next = world.GetNeighbour(cur, dir);
            if (cost_so_far.find(next) == cost_so_far.end() ||
                    new_cost < cost_so_far[next])
            {
                cost_so_far[next] = new_cost;
                frontier.push({ next, new_cost + heuristic(next) });
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

inline MapPoint GetClosest(
        const MapBase& world,
        const MapPoint& ref,
        const std::vector<MapPoint>& positions)
{
    MapPoint ret = MapPoint::Invalid();
    unsigned min = std::numeric_limits<unsigned>::max();

    for (const MapPoint& pos : positions) {
        unsigned dist = world.CalcDistance(ref, pos);
        if (dist < min) {
            min = dist;
            ret = pos;
        }
    }

    return ret;
}

inline std::vector<MapPoint> GetClosest(
        const MapBase& world,
        const MapPoint& ref,
        const std::vector<MapPoint>& positions,
        unsigned amount)
{
    typedef std::pair<MapPoint, unsigned> point_t;
    struct Less {
        bool operator()(const point_t& l, const point_t& r) {
            return l.second < r.second;
        }
    };
    std::priority_queue<point_t, std::vector<point_t>, Less> queue;

    for (const MapPoint& pos : positions) {
        unsigned dist = world.CalcDistance(ref, pos);
        queue.push({ pos, dist });
    }

    std::vector<MapPoint> ret;
    for (unsigned i = 0; i < amount && !queue.empty(); ++i) {
        ret.push_back(queue.top().first);
        queue.pop();
    }
    return ret;
}

} // namespace beowulf

#endif //! BEOWULF_HELPER_H_INCLUDED
