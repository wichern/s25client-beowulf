// Copyright (c) 2016 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "helper.h"
#include "ai/beowulf/Helper.h"
#include "RttrForeachPt.h"

bool ConstructBuilding(
        AIPlayer* ai,
        GameWorldGame& world,
        TestEventManager& em, BuildingType type,
        const MapPoint& pos,
        bool wait_for_site)
{
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::World& buildings = beowulf.world;

    beowulf::Building* bld = buildings.Create(
                type,
                beowulf::Building::PlanningRequest);
    buildings.Construct(bld, pos);

    if (!bld->GetPt().isValid()) return false;
    if (bld->GetType() != type) return false;
    if (bld->GetState() != beowulf::Building::ConstructionRequested) return false;

    if (!wait_for_site)
        return true;

    Proceed([&]() { return bld->GetState() == beowulf::Building::UnderConstruction; }, { ai }, em, world);

    if (!bld->GetPt().isValid()) return false;
    if (bld->GetType() != type) return false;
    if (bld->GetState() != beowulf::Building::UnderConstruction) return false;

    return true;
}

bool CompareBuildingsWithWorld(
        AIPlayer* ai,
        GameWorldGame& world)
{
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::World& buildings = beowulf.world;

    /*
     * Check that all buildings of the world also exist in the buildings object.
     */
    std::vector<std::pair<MapPoint, BuildingType>> to_check;

    const nobHQ* hq = beowulf.GetAII().GetHeadquarter();
    if (hq)
        to_check.push_back({ hq->GetPos(), hq->GetBuildingType()} );

    for (const noBuildingSite* bld : beowulf.GetAII().GetBuildingSites())
        to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (const nobMilitary* bld : beowulf.GetAII().GetMilitaryBuildings())
        to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (const nobUsual* bld : beowulf.GetAII().GetBuildings(static_cast<BuildingType>(i)))
            to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (const std::pair<MapPoint, BuildingType>& existing : to_check) {
        beowulf::Building* bld = buildings.GetBuilding(existing.first);
        if (bld == nullptr)
            return false;
        if (bld->GetType() != existing.second)
            return false;
        if (beowulf.GetAII().gwb.GetNO(existing.first)->GetType() == NOP_BUILDINGSITE) {
            if (bld->GetState() != beowulf::Building::UnderConstruction)
                return false;
        } else {
            if (bld->GetState() != beowulf::Building::Finished)
                return false;
        }
    }

    /*
     * Check that all buildings of the buildings object also exist in the world.
     */
    for (beowulf::Building* bld : buildings.GetBuildings()) {
        NodalObjectType type = world.GetNO(bld->GetPt())->GetType();

        if (bld->GetState() == beowulf::Building::UnderConstruction && type != NOP_BUILDINGSITE)
            return false;
        if (bld->GetState() == beowulf::Building::Finished && type != NOP_BUILDING)
            return false;
    }

    /*
     * Check all flags and roads
     */
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        bool flag_exists = world.GetNO(pt)->GetType() == NOP_FLAG;
        bool flag_known = buildings.GetFlagState(pt) == beowulf::FlagFinished;
        if (flag_exists != flag_known)
            return false;

        for (unsigned dir = 3; dir < Direction::COUNT; ++dir) {
            bool road_exist = world.GetRoad(pt, RoadDir(dir - 3)) != PointRoad::None;
            bool road_known = buildings.GetRoadState(pt, Direction(dir)) == beowulf::RoadFinished;
            if (road_exist != road_known) {
                if (world.GetNO(pt)->GetType() == NOP_BUILDING)
                    continue;
                if (world.GetNO(pt)->GetType() == NOP_BUILDINGSITE)
                    continue;
                if (world.GetNO(world.GetNeighbour(pt, Direction(dir)))->GetType() == NOP_BUILDING)
                    continue;
                if (world.GetNO(world.GetNeighbour(pt, Direction(dir)))->GetType() == NOP_BUILDINGSITE)
                    continue;
                return false;
            }
        }
    }

    return true;
}

bool IsConnected(const MapPoint& src, const MapPoint& dst, const beowulf::World& world)
{
    return beowulf::FindPath(src, world, nullptr,
    // Condition
    [&world, src](const MapPoint& pt, Direction dir)
    {
        if (pt == src && dir == Direction::NORTHWEST)
            return false;
        return world.HasRoad(pt, dir);
    },
    // End
    [dst](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == dst;
    },
    // Heuristic
    [&world, dst](const MapPoint& pt)
    {
        return world.CalcDistance(pt, dst);
    },
    // Cost
    [](const MapPoint& pt, Direction dir)
    {
        (void)pt;
        (void)dir;
        return 1;
    });
}

bool IsConnected(const beowulf::Building* building, const beowulf::Beowulf* beowulf)
{
    return IsConnected(building->GetFlag(), beowulf->world.GetHQFlag(), beowulf->world);
}

void Proceed(std::vector<AIPlayer*> player, TestEventManager& em, GameWorldGame& world)
{
    bool isnfw = false;
    while (!isnfw) {
        isnfw = em.GetCurrentGF() % 10;

        if (isnfw) {
            for (AIPlayer* p : player) {
                for (gc::GameCommandPtr gc : p->FetchGameCommands())
                    gc->Execute(world, p->GetPlayerId());
            }
        }

        for (AIPlayer* p : player)
            p->RunGF(em.GetCurrentGF(), isnfw);

        em.ExecuteNextEvent(em.GetCurrentGF() + 1);
    }
}

bool IsOutsidePlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& points)
{
    for (const MapPoint& pt : points) {
        if (beowulf->world.IsPlayerTerritory(pt, true))
            return false;
    }
    return true;
}

bool IsInsidePlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& points)
{
    for (const MapPoint& pt : points) {
        if (!beowulf->world.IsPlayerTerritory(pt, true))
            return false;
    }
    return true;
}

std::vector<MapPoint> GetPlayerTerritory(
        const beowulf::Beowulf* beowulf)
{
    std::vector<MapPoint> ret;
    RTTR_FOREACH_PT(MapPoint, beowulf->world.GetSize()) {
        if (beowulf->world.IsPlayerTerritory(pt, true)) {
            ret.push_back(pt);
        }
    }
    return ret;
}

bool EqualsPlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& originalPoints,
        const std::vector<MapPoint>& additionalPoints)
{
    for (const MapPoint& pt : GetPlayerTerritory(beowulf)) {
        if (beowulf->world.IsPlayerTerritory(pt, true)) {
            if (std::find(originalPoints.begin(), originalPoints.end(), pt) != originalPoints.end())
                continue;
            else if (std::find(additionalPoints.begin(), additionalPoints.end(), pt) == additionalPoints.end())
                return false;
        }
    }
    if (!IsInsidePlayerTerritory(beowulf, additionalPoints))
        return false;
    return true;
}
