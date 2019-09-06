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

#include "rttrDefines.h" // IWYU pragma: keep
#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

#ifdef BEOWULF_ENABLE_ALL

BOOST_AUTO_TEST_SUITE(BeowulfBuildLocations)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(BuildLocationsEmptyMap, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    const beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);

    {
        bl.Calculate(beowulf.world, BiggerWorld_HQFlag);

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

    // Calling Calculate() a second time.
    {
        bl.Calculate(beowulf.world, BiggerWorld_HQFlag);

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

    // Create a territory with a non reachable section:
    {
        world.SetNO({13, 3}, new noTree({13, 3}, 0, 3));
        world.SetNO({14, 4}, new noTree({14, 4}, 0, 3));
        world.SetNO({14, 5}, new noTree({14, 5}, 0, 3));
        world.SetNO({15, 6}, new noTree({15, 6}, 0, 3));
        world.SetNO({15, 7}, new noTree({15, 7}, 0, 3));
        world.SetNO({16, 7}, new noTree({16, 7}, 0, 3));
        world.SetNO({17, 7}, new noTree({17, 7}, 0, 3));
        world.SetNO({18, 7}, new noTree({18, 7}, 0, 3));

        bl.Update(beowulf.world, {13, 3}, 2);
        bl.Update(beowulf.world, {14, 4}, 2);
        bl.Update(beowulf.world, {14, 5}, 2);
        bl.Update(beowulf.world, {15, 6}, 2);
        bl.Update(beowulf.world, {15, 7}, 2);
        bl.Update(beowulf.world, {16, 7}, 2);
        bl.Update(beowulf.world, {17, 7}, 2);
        bl.Update(beowulf.world, {18, 7}, 2);

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 157);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 166);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 502);
    }
}

BOOST_FIXTURE_TEST_CASE(PlaceFarmsClosely, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint farm1Point(9, 11);
    MapPoint farm2Point(7, 11);
    MapPoint farm3Point(6, 11);

    BOOST_REQUIRE(beowulf.world.GetBQ(farm1Point) >= BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm2Point) >= BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm3Point) >= BQ_CASTLE);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.world, MapPoint(13, 12)); // HQ flag
    BOOST_REQUIRE(bl.Get(farm1Point) >= BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) >= BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);

    beowulf::Building* farm1 = beowulf.world.Create(BLD_FARM, beowulf::Building::PlanningRequest);
    beowulf.world.Construct(farm1, farm1Point);

    BOOST_REQUIRE(beowulf.world.GetBQ(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm3Point) >= BQ_CASTLE);
    bl.Update(beowulf.world, farm1Point);
    BOOST_REQUIRE(bl.Get(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(beowulf.world.GetBQ(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(beowulf.world.GetBQ(farm3Point) >= BQ_CASTLE);
    bl.Update(beowulf.world, farm1Point);
    BOOST_REQUIRE(bl.Get(farm1Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm2Point) < BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(farm3Point) >= BQ_CASTLE);
}

BOOST_FIXTURE_TEST_CASE(UpdateBuildLocations, BiggerWorldWithGCExecution)
{
    /*
     * We place a couple of construction sites and check whether BQC and BuildLocations map perfectly.
     */
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.world, MapPoint(13, 12)); // HQ flag

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);
        BuildingQuality bq_GWB = beowulf.GetAIInterface().gwb.GetBQ(pt, beowulf.GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags
        if (bq_GWB != BQ_FLAG) {
            BOOST_REQUIRE(bq_GWB == bq_BuildLocations);
        }
    }

    MapPoint farmPoint(9, 11);
    MapPoint wellPoint(7, 11);
    MapPoint millPoint(11, 13);

    beowulf.world.Construct(beowulf.world.Create(BLD_FARM, beowulf::Building::PlanningRequest), farmPoint);
    bl.Update(beowulf.world, farmPoint);
    beowulf.world.Construct(beowulf.world.Create(BLD_WELL, beowulf::Building::PlanningRequest), wellPoint);
    bl.Update(beowulf.world, wellPoint);
    beowulf.world.Construct(beowulf.world.Create(BLD_MILL, beowulf::Building::PlanningRequest), millPoint);
    bl.Update(beowulf.world, millPoint);

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(beowulf.world.GetBuildings(farmPoint)->GetState() == beowulf::Building::UnderConstruction);
    BOOST_REQUIRE(beowulf.world.GetBuildings(wellPoint)->GetState() == beowulf::Building::UnderConstruction);
    BOOST_REQUIRE(beowulf.world.GetBuildings(millPoint)->GetState() == beowulf::Building::UnderConstruction);

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);
        BuildingQuality bq_GWB = beowulf.GetAIInterface().gwb.GetBQ(pt, beowulf.GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags
        if (bq_GWB != BQ_FLAG) {
            BOOST_REQUIRE(bq_GWB == bq_BuildLocations);
        }
    }

    // Place Road
    std::vector<Direction> route = { Direction::EAST, Direction::EAST, Direction::EAST,
                                     Direction::NORTHWEST, Direction::NORTHWEST, Direction::WEST };
    //std::vector<Direction> route = { Direction::NORTHEAST, Direction::NORTHEAST };
    beowulf.world.ConstructRoad(MapPoint(12, 14), route);

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.write();

    bl.Update(beowulf.world, MapPoint(12, 14), route.size());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            if (bq_Beowulf != bq_BuildLocations) {
                BOOST_REQUIRE(false);
            }
        }
    }

    Proceed(ai, world, curPlayer, em);

    bl.Update(beowulf.world, MapPoint(12, 14), route.size());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);
        BuildingQuality bq_GWB = beowulf.GetAIInterface().gwb.GetBQ(pt, beowulf.GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags
        if (bq_GWB != BQ_FLAG) {
            BOOST_REQUIRE(bq_GWB == bq_BuildLocations);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
