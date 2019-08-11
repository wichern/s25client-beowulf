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
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/BuildingQualityCalculator.h"

#include "nodeObjs/noTree.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfBuildingsPlan)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(PlanBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.buildings, MapPoint(13, 12)); // HQ flag

    beowulf::Building* bld = beowulf.buildings.Create(BLD_CATAPULT, beowulf::Building::PlanningRequest);
    beowulf.buildings.Plan(bld, MapPoint(16,13));
    bl.Update(beowulf.buildings.GetBQC(), MapPoint(16,13), 2);

    // Sampling some effects
    BOOST_REQUIRE(bl.Get(MapPoint(16,13)) == BQ_NOTHING);
    BOOST_REQUIRE(bl.Get(MapPoint(15,13)) == BQ_NOTHING);
    BOOST_REQUIRE(bl.Get(MapPoint(14,13)) == BQ_HOUSE);
    BOOST_REQUIRE(bl.Get(MapPoint(13,13)) == BQ_HOUSE);
    BOOST_REQUIRE(bl.Get(MapPoint(14,15)) == BQ_CASTLE);
}

BOOST_FIXTURE_TEST_CASE(PlanRoadSouthEast, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildingQualityCalculator bqc(beowulf.GetAIInterface());
    bqc.AddBlockingReason(&beowulf.buildings);
    bqc.AddRoadProvider(&beowulf.buildings);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.buildings, MapPoint(13, 12)); // HQ flag

    beowulf.buildings.PlanRoad(MapPoint(13,12), {Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::SOUTHEAST});
    bl.Update(bqc, MapPoint(13,12), 4);

    //beowulf::CreateSvg(beowulf.GetAIInterface(), bl, "test2.svg");
}

BOOST_FIXTURE_TEST_CASE(PlanRoadWest, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.buildings, MapPoint(13, 12)); // HQ flag

    beowulf.buildings.PlanRoad(MapPoint(13,12), {Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST});
    bl.Update(beowulf.buildings.GetBQC(), MapPoint(13,12), 5);

    // BQ northwest of flag should be buildable.
    BOOST_REQUIRE(bl.Get(MapPoint(7,11)) == BQ_CASTLE);
    BOOST_REQUIRE(bl.Get(MapPoint(8,12)) == BQ_NOTHING);
}

BOOST_FIXTURE_TEST_CASE(IsRoadPossible, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.write();

    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(13, 12), Direction::SOUTHEAST));
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(world.GetNeighbour(MapPoint(13, 12), Direction::SOUTHEAST), Direction::NORTHWEST));
    BOOST_REQUIRE(!beowulf.buildings.IsRoadPossible(MapPoint(13, 12), Direction::NORTHWEST));
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(13, 12), Direction::WEST));
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(15, 15), Direction::WEST));
    BOOST_REQUIRE(!beowulf.buildings.IsRoadPossible(MapPoint(11, 11), Direction::EAST));
    BOOST_REQUIRE(!beowulf.buildings.IsRoadPossible(MapPoint(21, 10), Direction::EAST));
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(10, 13), Direction::NORTHWEST));

    beowulf.buildings.PlanRoad(MapPoint(13,12), {Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST});

//    map.draw(beowulf.buildings);
//    map.write();

    // there is currently no flag at 10,12 but we could add one.
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(10, 13), Direction::NORTHWEST));

    // there is currently no flag at 9,12 and we could NOT add one.
    BOOST_REQUIRE(!beowulf.buildings.IsRoadPossible(MapPoint(9, 13), Direction::NORTHWEST));

    BOOST_REQUIRE(!beowulf.buildings.IsRoadPossible(MapPoint(11, 12), Direction::EAST));
    BOOST_REQUIRE(beowulf.buildings.IsRoadPossible(MapPoint(9, 11), Direction::EAST));
}

BOOST_AUTO_TEST_SUITE_END()

#endif
