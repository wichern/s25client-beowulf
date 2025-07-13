// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/RoadBuilder.h"
#include "ai/AIInterface.h"
#include "pathfinding/PathConditionRoad.h"

namespace beowulf {

BuildLocations::BuildLocations(AIInterface& aii)
: aii_(aii)
{
    map_.Resize(aii_.gwb.GetSize());
}

BuildLocations::~BuildLocations()
{

}

void BuildLocations::Calculate(const MapPoint& start, bool confirm)
{
    RTTR_Assert(start.isValid());

    if (map_[start].visited)
        return;

    RoadBuilder roads(aii_);

    FloodFill(map_, start,
    // condition
    [&](const MapPoint& pt, Direction dir)
    {
        // Check if there is a road already OR we could build one.
        return roads.HasRoad(pt, dir) || roads.IsRoadPossible(pt, dir);
    },
    // action
    [&](const MapPoint& pt)
    {
        map_[pt].visited = true;
        BuildingQuality bq = aii_.gwb.GetBQ(pt, aii_.GetPlayerId());
        if (bq > BuildingQuality::Flag) {
            if (!confirm) {
                locations_.push_back({ pt, BuildingQuality::House });
                return;
            }
            RoadBuilder roadsPreview(aii_, pt, bq);
            // Check if we can still connect that connection if we place a building.
            MapPoint flag = aii_.gwb.GetNeighbour(pt, Direction::SouthEast);
            if (roadsPreview.CanConnect(flag, start)) {
                locations_.push_back({ pt, bq });
            } else if (bq == BuildingQuality::Castle) {
                RoadBuilder roadsPreview2(aii_, pt, BuildingQuality::House);
                // maybe we could connect a smaller building
                if (roadsPreview2.CanConnect(flag, start))
                    locations_.push_back({ pt, BuildingQuality::House });
            }
        }
    });
}

std::vector<MapPoint> BuildLocations::Get(BuildingQuality minBq) const
{
    std::vector<MapPoint> ret;

    for (auto const& [pt, bq] : locations_)
        if (bq >= minBq)
            ret.push_back(pt);

    return ret;
}

unsigned BuildLocations::GetSum() const
{
    // @todo: this rating function should not really belong here.
    unsigned sum = 0u;

    for (auto const& [pt, bq] : locations_)
    {
        switch (bq) {
        case BuildingQuality::Hut:
        case BuildingQuality::Mine:
            sum += 1;
            break;
        case BuildingQuality::House:
            sum += 2;
            break;
        case BuildingQuality::Castle:
            sum += 3;
            break;
        case BuildingQuality::Harbor:
            sum += 4;
            break;
        default:
            break;
        }
    }

    return sum;
}

unsigned BuildLocations::GetSize() const
{
    return locations_.size();
}

} // namespace beowulf