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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/BuildingPlannerBase.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/ResourceMap.h"
#include "ai/beowulf/Helper.h"

#include "ai/AIInterface.h"

namespace beowulf {

BuildingPlannerBase::BuildingPlannerBase(
        AIInterface& aii,
        Buildings& buildings,
        ResourceMap& resources,
        rnet_id_t rnet)
    : aii_(aii),
      buildings_(buildings),
      resources_(resources),
      locations_(aii.gwb),
      costs_(aii, resources_, buildings_),
      rnet_(rnet)
{
    locations_.Calculate(buildings, buildings.GetFlag(rnet));
}

BuildingPlannerBase::~BuildingPlannerBase()
{

}

double HyperVolume(const std::vector<double>& vec)
{
    double ret = 1.0;
    for (double v : vec)
        ret *= v;
    return ret;
}

bool BuildingPlannerBase::Place(
        Building* building,
        MapPoint& pt,
        std::vector<Direction>& route,
        bool construct)
{
    if (building->GetPt().isValid()) {
        if (!canUseBq(locations_.Get(building->GetPt()), building->GetQuality()))
            return false;
        pt = building->GetPt();
    } else {
        if (!FindBestPosition(building, pt, HyperVolume))
            return false;
    }

    if (construct)
        buildings_.Construct(building, pt);
    else
        buildings_.Plan(building, pt);

    MapPoint flagPos = aii_.gwb.GetNeighbour(pt, Direction::SOUTHEAST);
    MapPoint goodsDest = MapPoint::Invalid();
    Building* goodsDestBld = buildings_.GetGoodsDest(building, rnet_, pt);
    if (goodsDestBld)
        goodsDest = goodsDestBld->GetPt();
    if (!FindBestRoute(flagPos, goodsDest, route)) {
        // @todo: Find a second best building position and try there.
        locations_.Update(buildings_.GetBQC(), pt);
        return false;
    }

    if (!route.empty()) {
        if (construct)
            buildings_.ConstructRoad(flagPos, route);
        else
            buildings_.PlanRoad(flagPos, route);
        locations_.Update(buildings_.GetBQC(), flagPos, route.size() + 1);
    } else {
        locations_.Update(buildings_.GetBQC(), pt);
    }

    return true;
}

bool BuildingPlannerBase::FindBestRoute(
        const MapPoint& start,
        const MapPoint& ,
        std::vector<Direction>& route)
{
    // We might already be connected and we can assume that it is impossible to
    // be connected to another island than the one we want to.
    if (buildings_.IsPointConnected(start)) {
        route.clear();
        return true;
    }

    /*
     * We use an A* star to find a path.
     *
     * The search will terminate when it has reached a flag or a position on
     * an existing road where we can put a flag (of the same island).
     */
    return FindPath(start, aii_.gwb, /*goodsDest,*/ &route,
    // Condition
    [this, start](const MapPoint& pos, Direction dir)
    {
        if (pos == start && dir == Direction::NORTHWEST)
            return false;

        return buildings_.IsRoadPossible(pos, dir);
    },
    // End
    [&](const MapPoint& pos)
    {
        // The search can end if we found a way to any flag of the destination
        // island.
        return buildings_.GetRoadNetwork(pos) == rnet_;
    },
    // Heuristic
    [&](const MapPoint&)
    {
        return 0;
    },
    // Cost
    [](const MapPoint&, Direction)
    {
        return 1;
    });
}

} // namespace beowulf
