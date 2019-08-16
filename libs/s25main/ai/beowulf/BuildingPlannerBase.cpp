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
#include "ai/beowulf/World.h"
#include "ai/beowulf/Resources.h"
#include "ai/beowulf/Helper.h"

#include "ai/AIInterface.h"

#include "ai/beowulf/Debug.h"
#include "gameData/BuildingConsts.h"

namespace beowulf {

BuildingPlannerBase::BuildingPlannerBase(
        AIInterface& aii,
        World& buildings,
        Resources& resources,
        rnet_id_t rnet)
    : aii_(aii),
      world_(buildings),
      resources_(resources),
      locations_(aii.gwb),
      costs_(aii, resources_, world_),
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
        world_.Construct(building, pt);
    else
        world_.Plan(building, pt);

    MapPoint flagPos = aii_.gwb.GetNeighbour(pt, Direction::SOUTHEAST);
    MapPoint goodsDest = MapPoint::Invalid();
    Building* goodsDestBld = world_.GetGoodsDest(building, rnet_, pt);
    if (goodsDestBld)
        goodsDest = goodsDestBld->GetFlag();
    else
        goodsDest = aii_.gwb.GetNeighbour(world_.GetNearestBuilding(flagPos, { BLD_STOREHOUSE, BLD_HARBORBUILDING, BLD_HEADQUARTERS }, rnet_).first, Direction::SOUTHEAST);
    // still no dest? skip route building.
    if (!goodsDest.isValid()) {
        route.clear();
    } else {
        if (!FindBestRoute(flagPos, goodsDest, route)) {
            // @todo: Find a second best building position and try there.
            locations_.Update(world_, pt);
            return false;
        }
    }

    if (!route.empty()) {
        if (construct)
            world_.ConstructRoad(flagPos, route);
        else
            world_.PlanRoad(flagPos, route);
        locations_.Update(world_, flagPos, route.size() + 1);
    } else {
        locations_.Update(world_, pt);
    }

    return true;
}

bool BuildingPlannerBase::FindBestRoute(
        const MapPoint& start,
        const MapPoint& goodsDest,
        std::vector<Direction>& route)
{
    route.clear();

    // We might already be connected and we can assume that it is impossible to
    // be connected to another road network than the one we want to.
    if (world_.IsPointConnected(start)) {
        route.clear();
        return true;
    }

    /*
     * We use an A* star to find a path.
     *
     * The search will terminate when it has reached a flag or a position on
     * an existing road where we can put a flag (of the same road network).
     */
    std::vector<Direction> routeComplete;
    bool ret = FindPath(start, aii_.gwb, &routeComplete,
    // Condition
    [this, start](const MapPoint& pt, Direction dir)
    {
        if (pt == start && dir == Direction::NORTHWEST)
            return false;

        bool hasRoad = world_.HasRoad(pt, dir);
        bool roadPossible = world_.IsRoadPossible(pt, dir);
        return hasRoad || roadPossible;
    },
    // End
    [goodsDest](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == goodsDest;
    },
    // Heuristic
    [this, goodsDest](const MapPoint& pt)
    {
        return world_.CalcDistance(pt, goodsDest);
    },
    // Cost
    [this](const MapPoint& pt, Direction dir)
    {
        /* New roads cost '3' and existing roads cost '1'. */
        if (world_.HasRoad(pt, dir))
            return 1;
        return 3; // @todo: decrease cost when we can place a flag at dest
    });

    if (!ret)
        return ret;

    // Strip parts that already exist.
    MapPoint cur = start;
    for (Direction dir : routeComplete) {
        if (world_.HasRoad(cur, dir))
            break;
        route.push_back(dir);
        cur = aii_.gwb.GetNeighbour(cur, dir);
    }

    return ret;
}

} // namespace beowulf
