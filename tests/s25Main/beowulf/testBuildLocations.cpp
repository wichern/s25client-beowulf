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
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "nodeObjs/noGranite.h"

#include "RttrForeachPt.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfBuildLocations)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

using beowulf::Beowulf;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_FIXTURE_TEST_CASE(EmptyMap, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf::BuildLocations bl(beowulf_raw->world, false);

    // Counting found locations on an empty map.
    {
        bl.Calculate(beowulf_raw->world.GetHQFlag());

        // Check for duplicates
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Calling Calculate() a second time should lead to the same results.
    {
        bl.Calculate(beowulf_raw->world.GetHQFlag());

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Create a territory with a non reachable section.
    // Non reachable build locations should not be returned by BuildLocations.
    {
        world.SetNO({13, 3}, new noTree({13, 3}, 0, 3));
        world.SetNO({14, 4}, new noTree({14, 4}, 0, 3));
        world.SetNO({14, 5}, new noTree({14, 5}, 0, 3));
        world.SetNO({15, 6}, new noTree({15, 6}, 0, 3));
        world.SetNO({15, 7}, new noTree({15, 7}, 0, 3));
        world.SetNO({16, 7}, new noTree({16, 7}, 0, 3));
        world.SetNO({17, 7}, new noTree({17, 7}, 0, 3));
        world.SetNO({18, 7}, new noTree({18, 7}, 0, 3));

        bl.Update({13, 3}, 2);
        bl.Update({14, 4}, 2);
        bl.Update({14, 5}, 2);
        bl.Update({15, 6}, 2);
        bl.Update({15, 7}, 2);
        bl.Update({16, 7}, 2);
        bl.Update({17, 7}, 2);
        bl.Update({18, 7}, 2);

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

//        beowulf::AsciiMap map(beowulf_raw->GetAII());
//        map.draw(beowulf->world, true);
//        map.draw(bl);
//        map.write();

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 151);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 160);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 481);
    }
}

BOOST_FIXTURE_TEST_CASE(PlaceFarmsClosely, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    MapPoint farm1Point(9, 11);
    MapPoint farm2Point(7, 11);
    MapPoint farm3Point(6, 11);

    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm1Point, false) >= BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm2Point, false) >= BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm3Point, false) >= BQ_CASTLE);

    beowulf::BuildLocations bl(beowulf_raw->world, false);
    bl.Calculate(beowulf_raw->world.GetHQFlag());
    BOOST_REQUIRE(bl.Get(farm1Point) >= BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) >= BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);

    beowulf::Building* farm1 = beowulf_raw->world.Create(BLD_FARM, beowulf::Building::PlanningRequest);
    beowulf_raw->world.Construct(farm1, farm1Point);

    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm1Point, false) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm2Point, false) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm3Point, false) >= BQ_CASTLE);
    bl.Update(farm1Point);
    BOOST_REQUIRE(bl.Get(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);

    Proceed({ beowulf.get() }, em, world);

    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm1Point, false) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm2Point, false) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf_raw->world.GetBQ(farm3Point, false) >= BQ_CASTLE);
    bl.Update(farm1Point);
    BOOST_REQUIRE(bl.Get(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);
}

void ValidateBuildLocations(
        const GameWorldGame& world,
        const Beowulf* beowulf,
        const beowulf::BuildLocations& bl,
        bool hasPlanned = false);
void ValidateBuildLocations(
        const GameWorldGame& world,
        const Beowulf* beowulf,
        const beowulf::BuildLocations& bl,
        bool hasPlanned)
{
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);
        BuildingQuality bq_GWB = beowulf->GetAII().gwb.GetBQ(pt, beowulf->GetPlayerId());

        if (!hasPlanned && bq_GWB != bq_Beowulf) {
            beowulf::AsciiMap map(beowulf->GetAII());
            map.draw(beowulf->world, true);
            map.draw(bl);
            map.write();
            BOOST_FAIL("bq_GWB != bq_Beowulf");
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            if (bq_Beowulf != bq_BuildLocations) {
                beowulf::AsciiMap map(beowulf->GetAII());
                map.draw(beowulf->world, true);
                map.draw(bl);
                map.write();
                BOOST_FAIL("bq_Beowulf != bq_BuildLocations");
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(UpdateBuildLocations, BiggerWorldWithGCExecution)
{
    /*
     * We place a couple of construction sites and check whether BQC and BuildLocations map perfectly.
     */
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());
    ValidateBuildLocations(world, beowulf_raw, bl);

    MapPoint farmPoint(9, 11);
    MapPoint wellPoint(7, 11);
    MapPoint millPoint(11, 13);

    beowulf_raw->world.Construct(beowulf_raw->world.Create(BLD_FARM, beowulf::Building::PlanningRequest), farmPoint);
    bl.Update(farmPoint);
    beowulf_raw->world.Construct(beowulf_raw->world.Create(BLD_WELL, beowulf::Building::PlanningRequest), wellPoint);
    bl.Update(wellPoint);
    beowulf_raw->world.Construct(beowulf_raw->world.Create(BLD_MILL, beowulf::Building::PlanningRequest), millPoint);
    bl.Update(millPoint);

    // We proceed until one building is under construction (exists in-game and occupies build locations).
    Proceed([&]() { return beowulf_raw->world.GetBuilding(farmPoint)->GetState() == beowulf::Building::UnderConstruction; }, { beowulf_raw }, em, world);

    BOOST_REQUIRE(beowulf_raw->world.GetBuilding(farmPoint)->GetState() == beowulf::Building::UnderConstruction);
    BOOST_REQUIRE(beowulf_raw->world.GetBuilding(wellPoint)->GetState() == beowulf::Building::UnderConstruction);
    BOOST_REQUIRE(beowulf_raw->world.GetBuilding(millPoint)->GetState() == beowulf::Building::UnderConstruction);
    ValidateBuildLocations(world, beowulf_raw, bl);

    // Place Road
    std::vector<Direction> route = { Direction::EAST, Direction::EAST, Direction::EAST,
                                     Direction::NORTHWEST, Direction::NORTHWEST, Direction::WEST };
    beowulf_raw->world.ConstructRoad(MapPoint(12, 14), route);

    bl.Update(MapPoint(12, 14), static_cast<unsigned>(route.size()));

    // Set hasPlanned to true, since we only requested to build, but the engine did not yet execute the command.
    ValidateBuildLocations(world, beowulf_raw, bl, true);

    Proceed([&]() { return beowulf_raw->GetAII().IsRoad({ 12, 14 }, Direction::EAST); }, { beowulf_raw }, em, world);

    bl.Update(MapPoint(12, 14), static_cast<unsigned>(route.size()));
    ValidateBuildLocations(world, beowulf_raw, bl);
}

BOOST_FIXTURE_TEST_CASE(BlockingManners, WorldLoaded1PFixture)
{
    /*
     * We place a couple of construction sites and check whether BQC and BuildLocations map perfectly.
     */
    //world.SetNO({18, 104}, new noTree({18, 104}, 0, 3));
    world.SetNO({18, 104}, new noGranite(GT_1, 1));
    world.RecalcBQAroundPointBig({18, 104});

    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

//    beowulf::AsciiMap map(beowulf_raw->GetAII(), beowulf_raw->world.GetHQ()->GetPt(), 12);
//    map.draw(beowulf_raw->world);
//    map.drawResources();
//    map.draw(bl);
//    map.write();

//    map.clear();
//    map.draw(beowulf_raw->world);
//    map.drawResources();
//    map.drawBuildLocations(beowulf->GetPlayerId());
//    map.write();

    ValidateBuildLocations(world, beowulf_raw, bl);
}

BOOST_FIXTURE_TEST_CASE(NotConnectableBuildLocations, BiggerWorldWithGCExecution)
{
    // We create a situation where some build locations become invalid because it is
    // no longer possible to connect them to the road network.

    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

//    beowulf::AsciiMap map(beowulf_raw->GetAII(), beowulf_raw->world.GetHQ()->GetPt(), 10);
//    map.draw(beowulf_raw->world, false);
//    map.draw(bl);
//    map.write();

    beowulf_raw->world.ConstructRoad(beowulf_raw->world.GetHQ()->GetFlag(), {
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::SOUTHWEST,
                                     Direction::SOUTHWEST,
                                     Direction::EAST,
                                     Direction::NORTHEAST
                                 });
    beowulf_raw->world.ConstructRoad({ 9, 12 }, {
                                     Direction::NORTHWEST,
                                     Direction::WEST,
                                     Direction::WEST,
                                     Direction::NORTHWEST,
                                     Direction::WEST,
                                     Direction::NORTHEAST,
                                     Direction::SOUTHEAST
                                 });
    beowulf_raw->world.ConstructRoad({ 7, 13 }, {
                                     Direction::SOUTHEAST,
                                     Direction::SOUTHWEST,
                                     Direction::WEST
                                 });

    bl.Update({ 13, 12 }, 10);

//    map.clear();
//    map.draw(beowulf->world, false);
//    map.draw(bl);
//    map.write();

    Proceed({ beowulf_raw }, em, world);

    beowulf::BuildLocations bl2(beowulf_raw->world, true);
    bl2.Calculate(beowulf_raw->world.GetHQFlag());

//    map.clear();
//    map.draw(beowulf_raw->world, false);
//    map.draw(bl2);
//    map.write();

    BOOST_REQUIRE(bl.Get({ 5, 11 }) == BQ_NOTHING);
    BOOST_REQUIRE(bl2.Get({ 5, 11 }) == BQ_NOTHING);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
