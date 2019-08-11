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

#if 0

#include "rttrDefines.h" // IWYU pragma: keep
#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfRoadIslands)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(HQFlagOnly, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    BOOST_REQUIRE(beowulf.buildings.GetRoadNetwork(MapPoint(13, 12)) != beowulf::InvalidRoadNetwork);
}

BOOST_FIXTURE_TEST_CASE(PlanAndUnplanFlag, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint newFlagPt(10, 10);

    beowulf.buildings.PlanFlag(newFlagPt);

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    beowulf::rnet_id_t hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    beowulf::rnet_id_t new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);

    BOOST_REQUIRE(hq_flag_id != beowulf::InvalidRoadNetwork);
    BOOST_REQUIRE(new_flag_id != beowulf::InvalidRoadNetwork);
    BOOST_REQUIRE(hq_flag_id != new_flag_id);

    beowulf.buildings.ClearPlan();

    new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id == beowulf::InvalidRoadNetwork);
}

BOOST_FIXTURE_TEST_CASE(PlanAndUnplanRoad, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint newFlagPt(10, 10);
    std::vector<Direction> route = { Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::EAST, Direction::EAST };

    beowulf.buildings.PlanFlag(newFlagPt);
    beowulf.buildings.PlanRoad(newFlagPt, route);

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    beowulf::rnet_id_t new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id != beowulf::InvalidRoadNetwork);

    MapPoint cur = newFlagPt;
    for (Direction dir : route) {
        BOOST_REQUIRE(new_flag_id == beowulf.buildings.GetRoadNetwork(cur));
        cur = world.GetNeighbour(cur, dir);
    }

    beowulf::rnet_id_t hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(hq_flag_id == new_flag_id);

    beowulf.buildings.ClearPlan();

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id == beowulf::InvalidRoadNetwork);
    hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(hq_flag_id != beowulf::InvalidRoadNetwork);
}

BOOST_FIXTURE_TEST_CASE(ConstructAndRemoveRoad, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint newFlagPt(10, 10);
    std::vector<Direction> route = { Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::EAST, Direction::EAST };

    beowulf.buildings.ConstructFlag(newFlagPt);
    beowulf.buildings.ConstructRoad(newFlagPt, route);

    for (int i = 0; i < 10; ++i) {
        Proceed(ai, world, curPlayer, em);
    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    beowulf::rnet_id_t new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id != beowulf::InvalidRoadNetwork);

    MapPoint cur = newFlagPt;
    for (Direction dir : route) {
        BOOST_REQUIRE(new_flag_id == beowulf.buildings.GetRoadNetwork(cur));
        cur = world.GetNeighbour(cur, dir);
    }

    beowulf::rnet_id_t hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(hq_flag_id == new_flag_id);

    beowulf.buildings.DeconstructFlag(newFlagPt);

    for (int i = 0; i < 10; ++i) {
        Proceed(ai, world, curPlayer, em);
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id == beowulf::InvalidRoadNetwork);
    hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(hq_flag_id != beowulf::InvalidRoadNetwork);
}

BOOST_FIXTURE_TEST_CASE(ConnectAndSeparateTwoIslands, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint newFlagPt(10, 10);
    std::vector<Direction> route = { Direction::SOUTHWEST, Direction::SOUTHEAST, Direction::WEST, Direction::SOUTHEAST };

    beowulf.buildings.ConstructFlag(newFlagPt);
    beowulf.buildings.ConstructRoad(newFlagPt, route);

    for (int i = 0; i < 10; ++i) {
        Proceed(ai, world, curPlayer, em);
    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface());
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    beowulf::rnet_id_t new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    BOOST_REQUIRE(new_flag_id != beowulf::InvalidRoadNetwork);

    MapPoint cur = newFlagPt;
    for (Direction dir : route) {
        BOOST_REQUIRE(new_flag_id == beowulf.buildings.GetRoadNetwork(cur));
        cur = world.GetNeighbour(cur, dir);
    }
    BOOST_REQUIRE(new_flag_id == beowulf.buildings.GetRoadNetwork(cur));
    beowulf::rnet_id_t hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(hq_flag_id != new_flag_id);

    // Connect with HQ
    std::vector<Direction> route2 = { Direction::EAST, Direction::EAST, Direction::EAST, Direction::NORTHEAST };
    beowulf.buildings.ConstructRoad(cur, route2);

    for (int i = 0; i < 10; ++i) {
        Proceed(ai, world, curPlayer, em);
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(new_flag_id == hq_flag_id);

    // Disconnect again
    beowulf.buildings.DeconstructRoad(cur, route2);

    for (int i = 0; i < 10; ++i) {
        Proceed(ai, world, curPlayer, em);
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(beowulf.buildings.GetRoadNetworks());
//    map.write();

    new_flag_id = beowulf.buildings.GetRoadNetwork(newFlagPt);
    hq_flag_id = beowulf.buildings.GetRoadNetwork(MapPoint(13, 12));
    BOOST_REQUIRE(new_flag_id != hq_flag_id);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
