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

void Proceed(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world,
        unsigned& player,
        TestEventManager& em)
{
    std::vector<gc::GameCommandPtr> aiGcs = ai->FetchGameCommands();
    for(unsigned i = 0; i < 5; i++)
    {
        em.ExecuteNextGF();
        ai->RunGF(em.GetCurrentGF(), i == 0);
    }
    for (gc::GameCommandPtr& gc : aiGcs)
        gc->Execute(world, player);
}

bool ConstructBuilding(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world, unsigned& player,
        TestEventManager& em, BuildingType type,
        const MapPoint& pos,
        bool wait_for_site)
{
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::World& buildings = beowulf.world;

    beowulf::Building* bld = buildings.Create(
                type,
                beowulf::Building::PlanningRequest,
                beowulf::InvalidProductionGroup);
    buildings.Construct(bld, pos);

    if (!bld->GetPt().isValid()) return false;
    if (bld->GetType() != type) return false;
    if (bld->GetState() != beowulf::Building::ConstructionRequested) return false;
    if (buildings.GetRoadNetworkId(bld->GetFlag()) == beowulf::InvalidRoadNetwork) return false;

    if (!wait_for_site)
        return true;

    while (bld->GetState() != beowulf::Building::UnderConstruction) {
        Proceed(ai, world, player, em);
    }

    if (!bld->GetPt().isValid()) return false;
    if (bld->GetType() != type) return false;
    if (bld->GetState() != beowulf::Building::UnderConstruction) return false;
    if (buildings.GetRoadNetworkId(bld->GetFlag()) == beowulf::InvalidRoadNetwork) return false;

    return true;
}

bool CompareBuildingsWithWorld(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world)
{
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::World& buildings = beowulf.world;

    /*
     * Check that all buildings of the world also exist in the buildings object.
     */
    std::vector<std::pair<MapPoint, BuildingType>> to_check;

    const nobHQ* hq = beowulf.GetAIInterface().GetHeadquarter();
    if (hq)
        to_check.push_back({ hq->GetPos(), hq->GetBuildingType()} );

    for (const noBuildingSite* bld : beowulf.GetAIInterface().GetBuildingSites())
        to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (const nobMilitary* bld : beowulf.GetAIInterface().GetMilitaryBuildings())
        to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (const nobUsual* bld : beowulf.GetAIInterface().GetBuildings(static_cast<BuildingType>(i)))
            to_check.push_back({ bld->GetPos(), bld->GetBuildingType()} );

    for (const std::pair<MapPoint, BuildingType>& existing : to_check) {
        beowulf::Building* bld = buildings.GetBuildings(existing.first);
        if (bld == nullptr)
            return false;
        if (bld->GetType() != existing.second)
            return false;
        if (beowulf.GetAIInterface().gwb.GetNO(existing.first)->GetType() == NOP_BUILDINGSITE) {
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
            bool road_exist = world.GetRoad(pt, dir - 3);
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
