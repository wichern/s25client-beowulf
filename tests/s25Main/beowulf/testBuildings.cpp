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
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Types.h"

#include "nodeObjs/noTree.h"
#include "nodeObjs/noFlag.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"

#include <boost/test/unit_test.hpp>

#include <memory> /* std::unique_ptr */

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfBuildings)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(InitialState, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    // Check that HQ has been added.
    BOOST_REQUIRE(buildings.GetBM(MapPoint(12, 11)) == BlockingManner::Building);
    BOOST_REQUIRE(buildings.GetBM(MapPoint(13, 12)) == BlockingManner::Flag);
    //BOOST_REQUIRE(buildings.GetBM(MapPoint(12, 13)) == BlockingManner::FlagsAround);

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(ConstructValidBuilding, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, true));
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    // Connect the building and wait for it to complete.
    buildings.ConstructRoad(flagPos, { Direction::EAST, Direction::EAST });

    beowulf::Building* bld = buildings.Get(buildPos);
    while (bld->GetState() != beowulf::Building::Finished) {
        Proceed(ai, world, curPlayer, em);
    }

    BOOST_REQUIRE(bld->GetState() == beowulf::Building::Finished);
    BOOST_REQUIRE(buildings.GetGoodsDest(buildings.GetWorld(), bld, beowulf::InvalidIsland, buildPos)->GetType() == BLD_HEADQUARTERS);

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(ConstructBuildingOnInvalidPosition, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint buildPos(11, 11);
    MapPoint flagPos(12, 12);

    ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, false);
    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(buildings.Get(buildPos) == nullptr);
    BOOST_REQUIRE(!buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructBuildingWhileStillRequested, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, false);
    buildings.Deconstruct(buildings.Get(buildPos));

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(buildings.Get(buildPos) == nullptr);
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructConstructionSite, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, true);
    buildings.Deconstruct(buildings.Get(buildPos));

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(buildings.Get(buildPos) == nullptr);
    // BOOST_REQUIRE(!buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructFinishedBuilding, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, true));

    // Connect the building and wait for it to complete.
    buildings.ConstructRoad(flagPos, { Direction::EAST, Direction::EAST });

    beowulf::Building* bld = buildings.Get(buildPos);
    while (bld->GetState() != beowulf::Building::Finished) {
        Proceed(ai, world, curPlayer, em);
    }

    buildings.Deconstruct(buildings.Get(buildPos));

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(buildings.Get(buildPos) == nullptr);

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructConnectedBuilding, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });

    // Construct the building whose flagpos is on the road.
    MapPoint buildPos(10, 11);
    MapPoint flagPos(11, 12);

    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, buildPos, true));

    buildings.Deconstruct(buildings.Get(buildPos));

    Proceed(ai, world, curPlayer, em);

    BOOST_REQUIRE(buildings.Get(buildPos) == nullptr);
    BOOST_REQUIRE(buildings.HasFlag(flagPos));

    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));

    // beowulf::CreateSvg(beowulf.GetAIInterface(), buildings, "test.svg");
}

BOOST_FIXTURE_TEST_CASE(ConstructFlags, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });

    BOOST_REQUIRE(buildings.GetIsland(MapPoint(9, 12)) == buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetFlags().size() == 2);

    /*
     * Place a flag on the road and check that it has the same island as the other flags.
     */
    buildings.ConstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) == buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetFlags().size() == 3);

    /*
     * Place a flag on a free spot and check that it has a new island.
     */
    buildings.ConstructFlag(MapPoint(10, 8));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(10, 8)) != buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(10, 8)) != beowulf::InvalidIsland);
    BOOST_REQUIRE(buildings.GetFlags().size() == 4);

    // check consistency
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
    Proceed(ai, world, curPlayer, em);
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(DeconstructFlags, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    // Create a road
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST, Direction::EAST, Direction::EAST });
    buildings.ConstructFlag(MapPoint(7, 12));
    buildings.ConstructRoad(MapPoint(7, 12), { Direction::EAST, Direction::EAST });

    BOOST_REQUIRE(buildings.GetIsland(MapPoint(9, 12)) == buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE_EQUAL(buildings.GetFlags().size(), 3);

    /*
     * Place a flag on the road and remove it.
     * This should remove the attached roads as well.
     */
    buildings.ConstructFlag(MapPoint(11, 12));
    Proceed(ai, world, curPlayer, em);
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));

    buildings.DeconstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(buildings.GetFlagState(MapPoint(11, 12)) == beowulf::FlagDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(11, 12), Direction::EAST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(12, 12), Direction::EAST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(11, 12), Direction::WEST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(10, 12), Direction::WEST) == beowulf::RoadDestructionRequested);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(9, 12), Direction::WEST) == beowulf::RoadFinished);
    BOOST_REQUIRE(buildings.GetRoadState(MapPoint(8, 12), Direction::WEST) == beowulf::RoadFinished);
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) == beowulf::InvalidIsland);

    Proceed(ai, world, curPlayer, em);
    //beowulf::CreateSvg(beowulf.GetAIInterface(), buildings, "test.svg");
    BOOST_REQUIRE_EQUAL(buildings.GetFlags().size(), 3);
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::EAST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(12, 12), Direction::EAST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::WEST));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(10, 12), Direction::WEST));
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(ConstructRoad, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    // Create a second island
    buildings.ConstructFlag(MapPoint(11, 12));
    buildings.ConstructFlag(MapPoint(9, 12));
    buildings.ConstructRoad(MapPoint(9, 12), { Direction::EAST, Direction::EAST });

    // Flags have different islands at first:
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) != buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(9, 12)) == buildings.GetIsland(MapPoint(11, 12)));

    // Constructing a road connects those islands
    buildings.ConstructRoad(MapPoint(11, 12), { Direction::EAST, Direction::EAST });
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) == buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(9, 12)) == buildings.GetIsland(MapPoint(13, 12)));

    // Compare state with world
    Proceed(ai, world, curPlayer, em);
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));

    // Deconstruct road leaves both flags with different islands again.
    buildings.DeconstructRoad(MapPoint(11, 12), { Direction::EAST, Direction::EAST });
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) != buildings.GetIsland(MapPoint(13, 12)));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(9, 12)) == buildings.GetIsland(MapPoint(11, 12)));

    // Compare state with world
    Proceed(ai, world, curPlayer, em);
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_FIXTURE_TEST_CASE(ConstructInvalidRoad, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    buildings.ConstructFlag(MapPoint(11, 12));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) != buildings.GetIsland(MapPoint(13, 12)));

    // Constructing a road connects those islands
    buildings.ConstructRoad(MapPoint(11, 12), { Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST });
    BOOST_REQUIRE(buildings.HasRoad(MapPoint(11, 12), Direction::NORTHEAST ));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) == buildings.GetIsland(MapPoint(13, 12)));

    Proceed(ai, world, curPlayer, em);
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
    BOOST_REQUIRE(!buildings.HasRoad(MapPoint(11, 12), Direction::NORTHEAST ));
    BOOST_REQUIRE(buildings.GetIsland(MapPoint(11, 12)) != buildings.GetIsland(MapPoint(13, 12)));
}

BOOST_FIXTURE_TEST_CASE(NewBuildingsAreAssignedToProductionGroups, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    MapPoint sawmillPos(10, 9);
    MapPoint woodcutterPos(9, 15);
    MapPoint storagePos(15, 13);

    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_SAWMILL, sawmillPos, false));
    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_WOODCUTTER, woodcutterPos, false));
    BOOST_REQUIRE(ConstructBuilding(ai, world, curPlayer, em, BLD_STOREHOUSE, storagePos, false));

    BOOST_REQUIRE(buildings.Get(sawmillPos)->GetGroup() != beowulf::InvalidProductionGroup);
    BOOST_REQUIRE(buildings.Get(sawmillPos)->GetGroup() == buildings.Get(woodcutterPos)->GetGroup());
    BOOST_REQUIRE(buildings.Get(storagePos)->GetGroup() != buildings.Get(woodcutterPos)->GetGroup());

    Proceed(ai, world, curPlayer, em);
    //beowulf::CreateSvg(beowulf.GetAIInterface(), buildings, "test.svg");
    BOOST_REQUIRE(CompareBuildingsWithWorld(ai, world));
}

BOOST_AUTO_TEST_SUITE_END()
