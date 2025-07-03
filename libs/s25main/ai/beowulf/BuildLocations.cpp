// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildLocations.h"

namespace beowulf {

BuildLocations::BuildLocations(const GameWorldBase& world, unsigned player)
: world_(world)
, player_(player)
{
    map_.Resize(world.GetSize());
}

BuildLocations::~BuildLocations()
{

}

void BuildLocations::Calculate(const MapPoint& start)
{
    RTTR_Assert(start.isValid());

    PathConditionRoad<GameWorldBase> roadChecker(world_, false);

    std::vector<MapPoint> queue;
    queue.push_back(start);
    map_[start].visited = true;

    while (!queue.empty()) {
        MapPoint cur = queue.back();
        queue.pop_back();

        // action(cur)
        BuildingQuality bq = world_.GetBQ(cur, player_);
        if (bq > BuildingQuality::Flag) {
            // Check if we can still connect that connection if we place a building.
            MapPoint flag = world_.GetNeighbour(cur, Direction::SouthEast);
            // Use FindPathForRoad with a dummy world (this?) that provides BM ()
            if (aii.FindFreePathForNewRoad(start->GetPos(), target->GetPos(), &route);)

            locations_.push_back({ cur, bq });
        }

        // Visit all neighbours
        for (const auto dir : helpers::EnumRange<Direction>{}) {
            MapPoint next = world_.GetNeighbour(cur, dir);
            if (map_[next].visited)
                continue;

            // Check if there is a road already or we could build one.
            if (world_.GetPointRoad(cur, dir) == PointRoad::None && !roadChecker.IsEdgeOk())
                continue;

            queue.push_back(next);
            map_[next].visited = true;
        }
    }
}

} // namespace beowulf