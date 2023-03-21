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
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/BuildLocations.h"

#include "nodeObjs/noTree.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

using beowulf::Beowulf;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_AUTO_TEST_SUITE(BeowulfBuildingsPlan)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(PlanBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

    beowulf::Building* bld = beowulf_raw->world.Create(BuildingType::Catapult, beowulf::Building::PlanningRequest);
    beowulf_raw->world.Plan(bld, MapPoint(16,13));
    bl.Update(MapPoint(16,13), 2);

    // Sampling some effects
    BOOST_REQUIRE(bl.Get(MapPoint(16,13)) == BuildingQuality::Nothing);
    BOOST_REQUIRE(bl.Get(MapPoint(15,13)) == BuildingQuality::Nothing);
    BOOST_REQUIRE(bl.Get(MapPoint(14,13)) == BuildingQuality::House);
    BOOST_REQUIRE(bl.Get(MapPoint(13,13)) == BuildingQuality::House);
    BOOST_REQUIRE(bl.Get(MapPoint(14,15)) == BuildingQuality::Castle);
}

BOOST_FIXTURE_TEST_CASE(PlanRoadSouthEast, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

    beowulf_raw->world.PlanRoad(MapPoint(13,12), {Direction::SouthEast, Direction::SouthEast, Direction::SouthEast, Direction::SouthEast});
    bl.Update(MapPoint(13,12), 4);
}

BOOST_FIXTURE_TEST_CASE(PlanRoadWest, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    beowulf::BuildLocations bl(beowulf_raw->world, true);
    bl.Calculate(beowulf_raw->world.GetHQFlag());

    beowulf_raw->world.PlanRoad(MapPoint(13,12), {Direction::West, Direction::West, Direction::West, Direction::West, Direction::West});
    bl.Update(MapPoint(13,12), 5);

    // BQ northwest of flag should be buildable.
    BOOST_REQUIRE(bl.Get(MapPoint(7,11)) == BuildingQuality::Castle);
    BOOST_REQUIRE(bl.Get(MapPoint(8,12)) == BuildingQuality::Nothing);
}

BOOST_FIXTURE_TEST_CASE(IsRoadPossible, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(13, 12), Direction::SouthEast, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(world.GetNeighbour(MapPoint(13, 12), Direction::SouthEast), Direction::NorthWest, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible(MapPoint(13, 12), Direction::NorthWest, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(13, 12), Direction::West, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(15, 15), Direction::West, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible(MapPoint(11, 11), Direction::East, true));
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible(MapPoint(21, 10), Direction::East, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(10, 13), Direction::NorthWest, true));

    beowulf_raw->world.PlanRoad(MapPoint(13,12), {Direction::West, Direction::West, Direction::West, Direction::West, Direction::West});

    // there is currently no flag at 10,12 but we could add one.
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(10, 13), Direction::NorthWest, true));

    // there is currently no flag at 9,12 and we could NOT add one.
    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible(MapPoint(9, 13), Direction::NorthWest, true));

    BOOST_REQUIRE(!beowulf_raw->world.IsRoadPossible(MapPoint(11, 12), Direction::East, true));
    BOOST_REQUIRE(beowulf_raw->world.IsRoadPossible(MapPoint(9, 11), Direction::East, true));
}

BOOST_AUTO_TEST_SUITE_END()

#endif
