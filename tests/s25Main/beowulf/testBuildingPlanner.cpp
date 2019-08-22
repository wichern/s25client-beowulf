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

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/BuildLocations.h"

#include "factories/AIFactory.h"
#include "buildings/noBuildingSite.h"

#include <boost/test/unit_test.hpp>

#include "helper.h"

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_AUTO_TEST_SUITE(BeowulfBuildingPlanner)

using beowulf::Beowulf;
using beowulf::Building;
using beowulf::rnet_id_t;
using beowulf::BuildLocations;

#ifdef BEOWULF_ENABLE_ALL

BOOST_FIXTURE_TEST_CASE(PlanSingleBuilding, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    Beowulf& beowulf = static_cast<Beowulf&>(*ai);

    Building* bld = beowulf.world.Create(BLD_BREWERY, Building::PlanningRequest);
    beowulf.RequestConstruction(bld, beowulf.world.GetRoadNetwork(BiggerWorld_HQFlag));

    while (bld->GetState() != Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }
}

BOOST_FIXTURE_TEST_CASE(PlanMultipleBuildings, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    Beowulf& beowulf = static_cast<Beowulf&>(*ai);

    std::vector<Building*> requests;
    requests.push_back(beowulf.world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FORESTER, Building::PlanningRequest));

    rnet_id_t rnet = beowulf.world.GetRoadNetwork(BiggerWorld_HQFlag);
    for (Building* building : requests)
        beowulf.RequestConstruction(building, rnet);

    while (requests.front()->GetState() != Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

    for (Building* building : requests) {
        BOOST_REQUIRE(building->GetState() == Building::UnderConstruction);
        BOOST_REQUIRE(beowulf.world.GetRoadNetwork(building->GetFlag()) == rnet);
    }

    // Can the building be finished?
    while (requests.front()->GetState() != Building::Finished) {
        Proceed(ai, world, curPlayer, em);
    }
    BOOST_REQUIRE(requests.front()->GetState() == Building::Finished);
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildingsStepByStep, BiggerWorldWithGCExecution)
{
    /**
     * Rerunning one many buildings planning step by step in order to find a bug.
     * This is still a useful unittest, as it covers the bug found.
     */
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    Beowulf& beowulf = static_cast<Beowulf&>(*ai);
    BuildLocations buildLocations(beowulf.world);
    BuildLocations buildLocationsCheck(beowulf.world);
    buildLocations.Calculate(beowulf.world, BiggerWorld_HQFlag);

    /*
     * Construct Sawmill at (14:12)
     * Construct route from (14:13) to (W, NW)
     * Construct Storehouse at (7:17)
     * Construct route from (8:18) to (NE, NE, NE, NE, NE, NE, E, E)
     * Construct Bakery at (8:15)
     * Construct Forester at (9:13)
     * Construct Farm at (12:15)
     * Construct route from (13:16) to (NE, NE, NE)
     * Construct Farm at (11:18)
     * Construct route from (11:19) to (NE, NE, NE)
     * Construct Farm at (6:11)
     * Construct route from (7:12) to (SE, SE, E, E)
     * Construct Mill at (11:16)
     * Construct route from (11:17) to (NE, E)
     * Construct Woodcutter at (11:14)
     * Construct route from (11:15) to (W, NW)
     * Construct Woodcutter at (12:13)
     * Construct route from (13:14) to (W, SW)
     * Construct Well at (7:14)
     * Construct route from (7:15) to (SE, E)
     *  0  .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     *
     *  1    .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     *
     *  2  .   .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  3    .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  4  .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .
     *
     *  5    .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .
     *
     *  6  .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .
     *
     *  7    .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .
     *
     *  8  .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .
     *
     *  9    .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .
     *
     *  10 .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .
     *
     *  11   .   .   .   _   _   _   Far _   _   _   _   _   HQ  _   _   _   _   _   _   _   _   _   .   .
     *
     *  12 .   .   .   .   _   _   _   0   _   _   _   0---0---0   Saw _   _   _   _   _   _   _   .   .
     *                                  \             /         \
     *  13   .   .   .   .   _   _   _   0   _   For 0   _   _   0---0   _   _   _   _   _   _   .   .   .
     *                                    \         /               /
     *  14 .   .   .   .   .   _   _   Wel 0---0---0   _   _   _   0   _   _   _   _   _   _   .   .   .
     *                                            /               /
     *  15   .   .   .   .   .   _   _   0   Bak 0   _   _   Far 0   _   _   _   _   _   _   .   .   .   .
     *                                    \     /               /
     *  16 .   .   .   .   .   .   _   _   0---0   _   _   _   0   _   _   _   _   _   _   .   .   .   .
     *                                        /               /
     *  17   .   .   .   .   .   .   _   Sto 0   _   _   _   0   _   _   _   _   _   _   .   .   .   .   .
     *                                      /               /
     *  18 .   .   .   .   .   .   .   _   0   _   _   Far 0   _   _   _   _   _   _   .   .   .   .   .
     *                                                    /
     *  19   .   .   .   .   .   .   .   _   _   _   _   0   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  20 .   .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  21   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     */

    beowulf::Building* sawmill = beowulf.world.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest);
    beowulf.world.Construct(sawmill, MapPoint(14, 12));
    beowulf.world.ConstructRoad(MapPoint(14, 13),
    { Direction::WEST, Direction::NORTHWEST });
    buildLocations.Update(beowulf.world, MapPoint(14, 13), 3);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* storehouse = beowulf.world.Create(BLD_STOREHOUSE, Building::PlanningRequest);
    beowulf.world.Construct(storehouse, MapPoint(7, 17));
    beowulf.world.ConstructRoad(MapPoint(8, 18),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(beowulf.world, MapPoint(8, 18), 8);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        if (bq_BuildLocations1 != bq_BuildLocations2) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* bakery = beowulf.world.Create(BLD_BAKERY, Building::PlanningRequest);
    beowulf.world.Construct(bakery, MapPoint(8, 15));
    buildLocations.Update(beowulf.world, MapPoint(8, 15));
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* forester = beowulf.world.Create(BLD_FORESTER, Building::PlanningRequest);
    beowulf.world.Construct(forester, MapPoint(9, 13));
    buildLocations.Update(beowulf.world, MapPoint(9, 13));
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* farm1 = beowulf.world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf.world.Construct(farm1, MapPoint(12, 15));
    beowulf.world.ConstructRoad(MapPoint(13, 16),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(beowulf.world, MapPoint(13, 16), 4);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        if (bq_BuildLocations1 != bq_BuildLocations2) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* farm2 = beowulf.world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf.world.Construct(farm2, MapPoint(11, 18));
    beowulf.world.ConstructRoad(MapPoint(11, 19),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(beowulf.world, MapPoint(11, 19), 4);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* farm3 = beowulf.world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf.world.Construct(farm3, MapPoint(6, 11));
    beowulf.world.ConstructRoad(MapPoint(7, 12),
    { Direction::SOUTHEAST,
      Direction::SOUTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(beowulf.world, MapPoint(7, 12), 5);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(beowulf.world, BiggerWorld_HQFlag);
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildings, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    Beowulf& beowulf = static_cast<Beowulf&>(*ai);

    std::vector<beowulf::Building*> requests;
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FARM, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_MILL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FORESTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_STOREHOUSE, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WELL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_BAKERY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FARM, Building::PlanningRequest));
    //requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));

    rnet_id_t island = beowulf.world.GetRoadNetwork(BiggerWorld_HQFlag);
    for (Building* bld : requests)
        beowulf.RequestConstruction(bld, island);

    while (requests.front()->GetState() != Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

    BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.world, BiggerWorld_HQFlag); // HQ flag

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

BOOST_FIXTURE_TEST_CASE(PlanManyBuildingsOnRealMap, WorldLoaded1PFixture)
{
    unsigned curPlayer = 0;

    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    Beowulf& beowulf = static_cast<Beowulf&>(*ai);

    std::vector<beowulf::Building*> requests;
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FARM, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_MILL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FORESTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_STOREHOUSE, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_WELL, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_BAKERY, Building::PlanningRequest));
    requests.push_back(beowulf.world.Create(BLD_FARM, Building::PlanningRequest));

    rnet_id_t island = beowulf.world.GetRoadNetwork(beowulf.world.Get().front()->GetFlag());
    BOOST_REQUIRE(island != beowulf::InvalidRoadNetwork);
    for (Building* bld : requests)
        beowulf.RequestConstruction(bld, island);

    while (requests.front()->GetState() != Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.drawResources(world);
//    map.draw(beowulf.world);
//    map.write();

    BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.world, beowulf.world.Get().front()->GetFlag());

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.world.GetBQ(pt);
        BuildingQuality bq_GWB = beowulf.GetAIInterface().gwb.GetBQ(pt, beowulf.GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags and can be same as gwb or none (because there is no route to the building flag)
        if (bq_GWB != BQ_FLAG && bq_GWB != bq_BuildLocations) {
            BOOST_REQUIRE(bq_BuildLocations == BQ_NOTHING);
        }
    }
}

#endif

BOOST_AUTO_TEST_SUITE_END()
