// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if 0

#    include "helpers/EnumRange.h"
#    include "world/MapBase.h"

#    include <vector>

/// @brief  Perform a simple breadth first search on a map.
/// condition ... bool f(const MapPoint& pos, Direction dir)
///               shall return true, when a transition is possible
/// action    ... void f(const MapPoint& pos)
///               called on every reachable position (including start pos)
template<typename Condition, typename Action>
void FloodFill(const MapBase& world, const MapPoint& start, Condition condition, Action action)
{
    std::vector<bool> visited;
    visited.resize(world.GetSize().x * world.GetSize().y);

    std::vector<MapPoint> pts;
    pts.push_back(start);
    visited[world.GetIdx(start)] = true;

    while(!pts.empty())
    {
        MapPoint cur = pts.back();
        pts.pop_back();

        action(cur);

        for(const auto dir : helpers::EnumRange<Direction>{})
        {
            MapPoint next = world.GetNeighbour(cur, dir);
            unsigned next_idx = world.GetIdx(next);
            if(!visited[next_idx] && condition(cur, dir))
            {
                pts.push_back(next);
                visited[next_idx] = true;
            }
        }
    }
}

#endif
