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

#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Types.h"

#include "nodeObjs/noTree.h"
#include "nodeObjs/noFlag.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"

#include "RttrForeachPt.h"

#include <boost/test/unit_test.hpp>

#include <memory> /* std::unique_ptr */
#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfBuildings)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;
typedef WorldWithGCExecution<1, 44, 42> EvenBiggerWorldWithGCExecution;

using beowulf::Beowulf;
using beowulf::Building;
using beowulf::InvalidProductionGroup;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_FIXTURE_TEST_CASE(InitialState, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    // Check that HQ has been added.
    beowulf::Building* building = buildings.GetBuilding(MapPoint(12, 11));
    BOOST_REQUIRE(building != nullptr);
    BOOST_REQUIRE(building->GetType() == BLD_HEADQUARTERS);
    BOOST_REQUIRE(buildings.HasFlag(MapPoint(13, 12)));

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf_raw, world));
}

BOOST_FIXTURE_TEST_CASE(ConstructValidBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf.get(), world, em, BLD_SAWMILL, buildPos, true));
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    // Connect the building and wait for it to complete.
    buildings.ConstructRoad(flagPos, { Direction::EAST, Direction::EAST });

    beowulf::Building* bld = buildings.GetBuilding(buildPos);
    Proceed([&](){ return bld->GetState() == beowulf::Building::Finished; }, { beowulf.get() }, em, world);

    BOOST_REQUIRE(bld->GetState() == beowulf::Building::Finished);
    BOOST_REQUIRE(buildings.GetGoodsDest(bld, buildPos)->GetType() == BLD_HEADQUARTERS);

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(ConstructBuildingOnInvalidPosition, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint buildPos(11, 11);
    MapPoint flagPos(12, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf.get(), world, em, BLD_SAWMILL, buildPos, false));
    Proceed({ beowulf.get() }, em, world);

    BOOST_REQUIRE(buildings.GetBuilding(buildPos) == nullptr);
    BOOST_REQUIRE(!buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructBuildingWhileStillRequested, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf_raw, world, em, BLD_SAWMILL, buildPos, false));
    buildings.Deconstruct(buildings.GetBuilding(buildPos));

    Proceed({ beowulf_raw }, em, world);

    BOOST_REQUIRE(buildings.GetBuilding(buildPos) == nullptr);
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf_raw, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructConstructionSite, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf_raw, world, em, BLD_SAWMILL, buildPos, true));
    buildings.Deconstruct(buildings.GetBuilding(buildPos));

    Proceed({ beowulf_raw }, em, world);

    BOOST_REQUIRE(buildings.GetBuilding(buildPos) == nullptr);
    // BOOST_REQUIRE(!buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf_raw, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructFinishedBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf_raw, world, em, BLD_SAWMILL, buildPos, true));

    // Connect the building and wait for it to complete.
    buildings.ConstructRoad(flagPos, { Direction::EAST, Direction::EAST });

    beowulf::Building* bld = buildings.GetBuilding(buildPos);
    Proceed([&]() { return bld->GetState() == beowulf::Building::Finished; }, { beowulf_raw }, em, world);

    buildings.Deconstruct(buildings.GetBuilding(buildPos));

    Proceed({ beowulf.get() }, em, world);

    BOOST_REQUIRE(buildings.GetBuilding(buildPos) == nullptr);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf_raw, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructConnectedBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });

    // Construct the building whose flagpos is on the road.
    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(beowulf_raw, world, em, BLD_SAWMILL, buildPos, true));

    buildings.Deconstruct(buildings.GetBuilding(buildPos));

    Proceed({ beowulf_raw }, em,  world);

    BOOST_REQUIRE(buildings.GetBuilding(buildPos) == nullptr);
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf_raw, world));

    // beowulf::CreateSvg(beowulf.GetAIInterface(), buildings, "test.svg");
}

BOOST_FIXTURE_TEST_CASE(ConstructFlags, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });

    BOOST_REQUIRE(IsConnected(MapPoint(9, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(buildings.GetFlags().size() == 2);

    /*
     * Place a flag on the road and check that it has the same island as the other flags.
     */
    buildings.ConstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(buildings.GetFlags().size() == 3);

    /*
     * Place a flag on a free spot and check that it has a new island.
     */
    buildings.ConstructFlag(MapPoint(10, 8));
    BOOST_REQUIRE(!IsConnected(MapPoint(10, 8), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(buildings.GetFlags().size() == 4);

    // check consistency
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructFlags, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });
    buildings.ConstructFlag(MapPoint(7, 12));
    buildings.ConstructRoad(MapPoint(7, 12), { Direction::EAST, Direction::EAST });

    BOOST_REQUIRE(IsConnected(MapPoint(9, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE_EQUAL(buildings.GetFlags().size(), 3);

    /*
     * Place a flag on the road and remove it.
     * This should remove the attached roads as well.
     */
    buildings.ConstructFlag(MapPoint(11, 12));
    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));

    buildings.DeconstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(buildings.GetFlagState(MapPoint(11, 12)) == beowulf::FlagDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(11, 12), Direction::EAST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(12, 12), Direction::EAST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(11, 12), Direction::WEST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(10, 12), Direction::WEST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(9, 12), Direction::WEST) == beowulf::RoadFinished);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(8, 12), Direction::WEST) == beowulf::RoadFinished);

    Proceed({ beowulf.get() }, em,  world);
    //beowulf::CreateSvg(beowulf.GetAIInterface(), buildings, "test.svg");
    BOOST_REQUIRE_EQUAL(buildings.GetFlags().size(), 3);
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::EAST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(12, 12), Direction::EAST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::WEST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(10, 12), Direction::WEST));
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(ConstructRoad, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    // Create a second island
    buildings.ConstructFlag(MapPoint(11, 12));
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST });

    // Flags have different islands at first:
    BOOST_REQUIRE(!IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(IsConnected(MapPoint(9, 12), MapPoint(11, 12), beowulf_raw->world));

    // Constructing a road connects those islands
    buildings.ConstructRoad(MapPoint(11, 12), { Direction::EAST, Direction::EAST });
    BOOST_REQUIRE(IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(IsConnected(MapPoint(9, 12), MapPoint(13, 12), beowulf_raw->world));

    // Compare state with world
    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));

    // Deconstruct road leaves both flags with different islands again.
    buildings.DeconstructRoad(MapPoint(11, 12), { Direction::EAST, Direction::EAST });
    BOOST_REQUIRE(!IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));
    BOOST_REQUIRE(IsConnected(MapPoint(9, 12), MapPoint(11, 12), beowulf_raw->world));

    // Compare state with world
    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(ConstructInvalidRoad, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    buildings.ConstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(!IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));

    // Constructing a road connects those islands
    buildings.ConstructRoad(MapPoint(11, 12), { Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST });
    BOOST_REQUIRE(buildings.HasRoad(MapPoint(11, 12), Direction::NORTHEAST ));
    BOOST_REQUIRE(IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));

    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::NORTHEAST ));
    BOOST_REQUIRE(!IsConnected(MapPoint(11, 12), MapPoint(13, 12), beowulf_raw->world));
}

BOOST_FIXTURE_TEST_CASE(IsRoadPossible, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf_raw->world.ConstructFlag({ 10, 12 });
    beowulf_raw->world.ConstructRoad({ 10, 12 }, { Direction::EAST, Direction::EAST, Direction::EAST });
    beowulf_raw->world.ConstructFlag({ 10, 14 });
    beowulf_raw->world.ConstructRoad({ 10, 14 }, { Direction::NORTHEAST, Direction::EAST, Direction::EAST, Direction::NORTHEAST });

//    beowulf::AsciiMap map(beowulf_raw->GetAII());
//    map.draw(beowulf_raw->world);
//    map.write();

    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({10, 12}, Direction::EAST, true)); // already exists
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({10, 12}, Direction::SOUTHEAST, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible({10, 12}, Direction::NORTHEAST, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({12, 12}, Direction::NORTHEAST, true)); // hits HQ
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({10, 13}, Direction::NORTHWEST, true));
}

BOOST_FIXTURE_TEST_CASE(NewBuildingsAreAssignedToProductionGroups, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::World& buildings = beowulf_raw->world;

    MapPoint sawmillPos(10, 9);
    MapPoint woodcutterPos(9, 15);
    MapPoint storagePos(15, 13);

    BOOST_REQUIRE(ConstructBuilding(beowulf.get(), world, em, BLD_SAWMILL, sawmillPos, false));
    BOOST_REQUIRE(ConstructBuilding(beowulf.get(), world, em, BLD_WOODCUTTER, woodcutterPos, false));
    BOOST_REQUIRE(ConstructBuilding(beowulf.get(), world, em, BLD_STOREHOUSE, storagePos, false));

    BOOST_REQUIRE(buildings.GetBuilding(sawmillPos)->GetGroup() != beowulf::InvalidProductionGroup);
    BOOST_REQUIRE(buildings.GetBuilding(sawmillPos)->GetGroup() == buildings.GetBuilding(woodcutterPos)->GetGroup());
    BOOST_REQUIRE(buildings.GetBuilding(storagePos)->GetGroup() != buildings.GetBuilding(woodcutterPos)->GetGroup());

    Proceed({ beowulf.get() }, em,  world);
    BOOST_REQUIRE(CompareBuildingsWithWorld(beowulf.get(), world));
}

BOOST_FIXTURE_TEST_CASE(IsRoadPossibleOnlyInsideTerritory, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible({ 6, 10}, Direction::WEST, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({ 4, 10}, Direction::WEST, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible({ 2, 10}, Direction::EAST, true));
}

BOOST_FIXTURE_TEST_CASE(NewMilitaryBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    RTTR_FOREACH_PT(MapPoint, beowulf_raw->world.GetSize()) {
        BOOST_REQUIRE(beowulf_raw->world.IsBorder(pt, true) == beowulf_raw->GetAII().IsBorder(pt));
        BOOST_REQUIRE(beowulf_raw->world.IsPlayerTerritory(pt, true) == world.IsPlayerTerritory(pt, beowulf->GetPlayerId() + 1));
    }

    // Place construction site
    Building* building = beowulf_raw->world.Create(BLD_BARRACKS, Building::PlanningRequest);
    beowulf_raw->world.Construct(building, { 10, 5 });
    beowulf_raw->world.ConstructRoad({ 11, 6 }, { Direction::SOUTHWEST, Direction::SOUTHWEST, Direction::SOUTHWEST, Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::EAST, Direction::EAST });

    // Check that the actual border did not change
    BOOST_REQUIRE(beowulf_raw->GetAII().IsBorder({ 8, 2 }));

    // Check that the planned border changed
    BOOST_REQUIRE(!beowulf_raw->world.IsBorder({ 8, 2 }, true));
    BOOST_REQUIRE(beowulf_raw->world.IsBorder({ 4, 2 }, true));

    struct Node {
        bool isborder = false;
        bool isterritory = false;
    };
    NodeMapBase<Node> border;
    border.Resize(beowulf_raw->world.GetSize());
    RTTR_FOREACH_PT(MapPoint, beowulf_raw->world.GetSize()) {
        border[pt] = {
            beowulf_raw->world.IsBorder(pt, true),
            beowulf_raw->world.IsPlayerTerritory(pt, true)
        };
    }

    // Finish construction
    Proceed([&](){ return building->GetCaptured(); }, { beowulf.get() }, em, world);

    // Check that the new border matches the planned border.
    RTTR_FOREACH_PT(MapPoint, beowulf_raw->world.GetSize()) {
        const Node& node = border[pt];
        if (beowulf_raw->GetAII().IsBorder(pt) != node.isborder) {
            BOOST_REQUIRE(false);
        }
        if (world.IsPlayerTerritory(pt, beowulf->GetPlayerId() + 1) != node.isterritory) {
            BOOST_REQUIRE(false);
        }
    }
}

bool CheckGetAdditionalTerritory(
        Beowulf* beowulf,
        GameWorldGame& world,
        TestEventManager& em,
        const MapPoint& pos,
        BuildingType type,
        bool draw = false);
bool CheckGetAdditionalTerritory(
        Beowulf* beowulf,
        GameWorldGame& world,
        TestEventManager& em,
        const MapPoint& pos,
        BuildingType type,
        bool draw)
{
    // Check what World assumes we will gain as territory:
    std::vector<MapPoint> additionalTerritory;
    std::vector<const noBaseBuilding*> destroyed;
    beowulf->world.PredictExpansionResults(pos, type, additionalTerritory, destroyed);
    BOOST_REQUIRE_EQUAL(destroyed.size(), 0u);

    beowulf::AsciiMap map(beowulf->GetAII());
    if (draw) {
        map.draw(beowulf->world, true);
        map.write();

        map.clear();
        map.draw(beowulf->world, true);
        map.drawAdditionalTerritory(additionalTerritory);
        map.write();
    }

    std::vector<MapPoint> territory = GetPlayerTerritory(beowulf);
    BOOST_REQUIRE(IsOutsidePlayerTerritory(beowulf, additionalTerritory));

    // Build the building
    beowulf::Building* bld = beowulf->world.Create(type, beowulf::Building::PlanningRequest);
    beowulf->world.Construct(bld, pos);

    if (draw) {
        map.clear();
        map.draw(beowulf->world, true);
        map.write();
    }

    // Check that player territory is now equal to territory + additionaTerritory.
    if (!EqualsPlayerTerritory(beowulf, territory, additionalTerritory))
        return false;

    Proceed([&]() { return bld->GetState() == Building::UnderConstruction; }, { beowulf }, em, world);
    if (!beowulf->roads.Connect(bld))
        return false;
    Proceed([&](){ return bld->GetCaptured(); }, { beowulf }, em, world);

    // Re-Check that player territory is now equal to territory + additionaTerritory.
    if (!EqualsPlayerTerritory(beowulf, territory, additionalTerritory))
        return false;

    beowulf->world.DeconstructFlag(bld->GetFlag());
    Proceed([&]() {
        // Wait for burning site removed.
        if (beowulf->world.GetBuilding(pos))
            return false;
        if (world.GetNO(pos)->GetType() == NOP_FIRE)
            return false;
        return true;
    }, { beowulf }, em, world);

    if (draw) {
        map.clear();
        map.draw(beowulf->world, true);
        map.write();
    }

    return true;
}

BOOST_FIXTURE_TEST_CASE(GetAdditionalTerritory, EvenBiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

    unsigned counter = 0;
    for (const MapPoint& loc : bl.Get(BQ_HUT)) {
        if (counter > 21) // we get out or resources afterwards.
            break;
        if (!beowulf_raw->world.CanBuildMilitary(loc))
            continue;
        if (!CheckGetAdditionalTerritory(beowulf_raw, world, em, loc, BLD_GUARDHOUSE)) {
            BOOST_REQUIRE(false);
        }
        counter++;
    }
}

typedef WorldFixture<CreateEmptyWorld, 2, 40, 30> EmptyWorldFixture2P40x30;
BOOST_FIXTURE_TEST_CASE(TwoHQsAdditionalTerritory, EmptyWorldFixture2P40x30)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    std::unique_ptr<AIPlayer> player2(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 1, world));
    Beowulf* player2_raw = static_cast<Beowulf*>(player2.get());

    beowulf_raw->DisableRecurrents();
    player2_raw->DisableRecurrents();

    MapPoint pos(17, 15);
    BuildingType type = BLD_GUARDHOUSE;

    // Check what World assumes we will gain as territory:
    std::vector<MapPoint> additionalTerritory;
    std::vector<const noBaseBuilding*> destroyed;
    beowulf_raw->world.PredictExpansionResults(pos, type, additionalTerritory, destroyed);
    BOOST_REQUIRE_EQUAL(destroyed.size(), 0u);

//    beowulf::AsciiMap map(beowulf->GetAIInterface());
//    map.draw(beowulf_raw->world);
//    map.draw(player2->world);
//    map.write();

//    map.clear();
//    map.draw(beowulf_raw->world);
//    map.draw(player2->world);
//    map.drawAdditionalTerritory(additionalTerritory);
//    map.write();

    std::vector<MapPoint> territory = GetPlayerTerritory(beowulf_raw);
    BOOST_REQUIRE(IsOutsidePlayerTerritory(beowulf_raw, additionalTerritory));

    // Build the building
    beowulf::Building* bld = beowulf_raw->world.Create(type, beowulf::Building::PlanningRequest);
    beowulf_raw->world.Construct(bld, pos);

//    map.clear();
//    map.draw(player2->world);
//    map.draw(beowulf_raw->world);
//    map.write();

    // Check that player territory is now equal to territory + additionaTerritory.
    BOOST_REQUIRE(EqualsPlayerTerritory(beowulf_raw, territory, additionalTerritory));

    Proceed([&]() { return bld->GetState() == Building::UnderConstruction; }, { beowulf_raw }, em, world);
    BOOST_REQUIRE(beowulf_raw->roads.Connect(bld));
    Proceed([&](){ return bld->GetCaptured(); }, { beowulf_raw }, em, world);

//    map.clear();
//    map.draw(beowulf_raw->world);
//    map.draw(player2->world);
//    map.write();

    // Re-Check that player territory is now equal to territory + additionaTerritory.
    BOOST_REQUIRE(EqualsPlayerTerritory(beowulf_raw, territory, additionalTerritory));
}

typedef WorldFixture<CreateEmptyWorld, 2, 48, 32> EmptyWorldFixture2P48x32;
BOOST_FIXTURE_TEST_CASE(MultipleEnemyBuildingsAdditionalTerritory, EmptyWorldFixture2P48x32)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    std::unique_ptr<AIPlayer> player2(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 1, world));
    Beowulf* player2_raw = static_cast<Beowulf*>(player2.get());

    beowulf_raw->DisableRecurrents();
    player2_raw->DisableRecurrents();

    // Build enemy buildings.
    {
        std::vector<Building*> player2buildings;
        player2buildings.push_back(player2_raw->world.Create(BLD_BARRACKS, Building::PlanningRequest));
        player2_raw->world.Construct(player2buildings.back(), { 29, 13 });
        player2buildings.push_back(player2_raw->world.Create(BLD_GUARDHOUSE, Building::PlanningRequest));
        player2_raw->world.Construct(player2buildings.back(), { 30, 20 });

        Proceed([&]() {
            for (Building* bld : player2buildings)
                if (bld->GetState() != Building::UnderConstruction)
                    return false;
            return true;
        }, { beowulf.get(), player2.get() }, em, world);
        for (Building* bld : player2buildings) {
            BOOST_REQUIRE(player2_raw->roads.Connect(bld));
        }

        Proceed([&](){
            for (Building* bld : player2buildings)
                if (!bld->GetCaptured())
                    return false;
            return true;
        }, { beowulf.get(), player2.get() }, em, world);
    }

    // Build civilian buildings.
    {
//        beowulf::AsciiMap map(beowulf_raw->GetAII());
//        map.draw(beowulf_raw->world);
//        map.draw(player2->world);
//        map.drawBuildLocations(player2->GetPlayerId());
//        map.write();

        std::vector<Building*> player2buildings;
        player2buildings.push_back(player2_raw->world.Create(BLD_WELL, Building::PlanningRequest));
        player2_raw->world.Construct(player2buildings.back(), { 24, 12 });
        player2buildings.push_back(player2_raw->world.Create(BLD_FARM, Building::PlanningRequest));
        player2_raw->world.Construct(player2buildings.back(), { 25, 17 });
        player2buildings.push_back(player2_raw->world.Create(BLD_CATAPULT, Building::PlanningRequest));
        player2_raw->world.Construct(player2buildings.back(), { 23, 20 });

        Proceed([&]() {
            for (Building* bld : player2buildings)
                if (bld->GetState() != Building::UnderConstruction)
                    return false;
            return true;
        }, { beowulf.get(), player2_raw }, em, world);
        for (Building* bld : player2buildings) {
            BOOST_REQUIRE(player2_raw->roads.Connect(bld));
        }

        Proceed([&](){
            for (Building* bld : player2buildings)
                if (bld->GetState() != Building::Finished)
                    return false;
            return true;
        }, { beowulf.get(), player2_raw }, em, world);
    }

    // Estimate additional territory of a new military building.
    MapPoint pos(19, 16);
    BuildingType type = BLD_FORTRESS;

    std::vector<MapPoint> additionalTerritory;
    std::vector<const noBaseBuilding*> destroyed;
    beowulf_raw->world.PredictExpansionResults(pos, type, additionalTerritory, destroyed);

    BOOST_REQUIRE_EQUAL(destroyed.size(), 3u);

//    beowulf::AsciiMap map(beowulf_raw->GetAII());
//    map.draw(beowulf_raw->world);
//    map.draw(player2->world);
//    map.drawAdditionalTerritory(additionalTerritory);
//    map.write();

    std::vector<MapPoint> territory = GetPlayerTerritory(beowulf_raw);
    BOOST_REQUIRE(IsOutsidePlayerTerritory(beowulf_raw, additionalTerritory));

    // Build the building
    beowulf::Building* bld = beowulf_raw->world.Create(type, beowulf::Building::PlanningRequest);
    beowulf_raw->world.Construct(bld, pos);

//    map.clear();
//    map.draw(beowulf_raw->world);
//    map.draw(player2->world);
//    map.write();

    // Check that player territory is now equal to territory + additionaTerritory.
    BOOST_REQUIRE(EqualsPlayerTerritory(beowulf_raw, territory, additionalTerritory));

    Proceed([&]() { return bld->GetState() == Building::UnderConstruction; }, { beowulf_raw }, em, world);
    BOOST_REQUIRE(beowulf_raw->roads.Connect(bld));
    Proceed([&](){ return bld->GetCaptured(); }, { beowulf_raw }, em, world);
}

// @todo: Test on capturing enemy buildings.

#endif

BOOST_AUTO_TEST_SUITE_END()
