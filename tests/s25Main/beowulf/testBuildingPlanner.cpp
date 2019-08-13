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

#if 1

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

BOOST_FIXTURE_TEST_CASE(PlanSingleBuilding, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    beowulf::Building* bld = buildings.Create(BLD_BREWERY, beowulf::Building::PlanningRequest);
    beowulf.RequestConstruction(bld, buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag()));

    while (bld->GetState() != beowulf::Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
    map.draw(beowulf.buildings);
    map.write();
}

BOOST_FIXTURE_TEST_CASE(PlanMultipleBuildings, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    std::vector<beowulf::Building*> requests;
    requests.push_back(buildings.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_FORESTER, beowulf::Building::PlanningRequest));

    beowulf::rnet_id_t island = buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag());
    for (beowulf::Building* bld : requests)
        beowulf.RequestConstruction(bld, island);

    while (requests.front()->GetState() != beowulf::Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
    map.drawResources(ai->gwb);
    map.draw(beowulf.buildings);
    map.write();
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildingsStepByStep, BiggerWorldWithGCExecution)
{
    /**
     * Rerunning one many buildings planning step by step in order to find a bug.
     * This is still a useful unittest, as it covers the bug found.
     */
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;
    beowulf::BuildLocations buildLocations(world);
    beowulf::BuildLocations buildLocationsCheck(world);
    buildLocations.Calculate(buildings, MapPoint(13, 12));

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

    beowulf::Building* sawmill = buildings.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest);
    buildings.Construct(sawmill, MapPoint(14, 12));
    buildings.ConstructRoad(MapPoint(14, 13),
    { Direction::WEST,
      Direction::NORTHWEST });
    buildLocations.Update(buildings.GetBQC(), MapPoint(14, 13), 3);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    beowulf::Building* storehouse = buildings.Create(BLD_STOREHOUSE, beowulf::Building::PlanningRequest);
    buildings.Construct(storehouse, MapPoint(7, 17));
    buildings.ConstructRoad(MapPoint(8, 18),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(buildings.GetBQC(), MapPoint(8, 18), 8);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    beowulf::Building* bakery = buildings.Create(BLD_BAKERY, beowulf::Building::PlanningRequest);
    buildings.Construct(bakery, MapPoint(8, 15));
    buildLocations.Update(buildings.GetBQC(), MapPoint(8, 15));
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    beowulf::Building* forester = buildings.Create(BLD_FORESTER, beowulf::Building::PlanningRequest);
    buildings.Construct(forester, MapPoint(9, 13));
    buildLocations.Update(buildings.GetBQC(), MapPoint(9, 13));
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    beowulf::Building* farm1 = buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest);
    buildings.Construct(farm1, MapPoint(12, 15));
    buildings.ConstructRoad(MapPoint(13, 16),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(buildings.GetBQC(), MapPoint(13, 16), 4);
    Proceed(ai, world, curPlayer, em);

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocationsCheck);
//    map.write();

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        if (bq_BuildLocations1 != bq_BuildLocations2) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    beowulf::Building* farm2 = buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest);
    buildings.Construct(farm2, MapPoint(11, 18));
    buildings.ConstructRoad(MapPoint(11, 19),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(buildings.GetBQC(), MapPoint(11, 19), 4);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();

    beowulf::Building* farm3 = buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest);
    buildings.Construct(farm3, MapPoint(6, 11));
    buildings.ConstructRoad(MapPoint(7, 12),
    { Direction::SOUTHEAST,
      Direction::SOUTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(buildings.GetBQC(), MapPoint(7, 12), 5);
    Proceed(ai, world, curPlayer, em);

    buildLocationsCheck.Calculate(buildings, MapPoint(13, 12));
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

//    map.clear();
//    map.draw(beowulf.buildings);
//    map.draw(buildLocations);
//    map.write();
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildings, BiggerWorldWithGCExecution)
{
    AI::Info ai_info(AI::BEOWULF, AI::HARD);
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
    beowulf::Buildings& buildings = beowulf.buildings;

    std::vector<beowulf::Building*> requests;
    requests.push_back(buildings.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_FORESTER, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_STOREHOUSE, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_QUARRY, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_QUARRY, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_MILL, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_BAKERY, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_WELL, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));
    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));

    beowulf::rnet_id_t island = buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag());
    for (beowulf::Building* bld : requests)
        beowulf.RequestConstruction(bld, island);

    while (requests.front()->GetState() != beowulf::Building::UnderConstruction) {
        Proceed(ai, world, curPlayer, em);
    }

    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);
    bl.Calculate(beowulf.buildings, MapPoint(13, 12)); // HQ flag

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf.buildings.GetBQC().GetBQ(pt);
        BuildingQuality bq_GWB = beowulf.GetAIInterface().gwb.GetBQ(pt, beowulf.GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags
        if (bq_GWB != BQ_FLAG) {
            BOOST_REQUIRE(bq_GWB == bq_BuildLocations);
        }
    }

    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
    map.draw(beowulf.buildings);
    map.draw(beowulf.buildings.GetRoadNetworks());
    map.write();
}

//BOOST_FIXTURE_TEST_CASE(PlanSingleBuilding, BiggerWorldWithGCExecution)
//{
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

//    beowulf::BuildingPlanner bp(beowulf.GetAIInterface(), beowulf.buildings, beowulf.resourceMap);
//    bp.Request(BLD_WELL);

//    bp.Init();

//    bool success = false;
//    for (unsigned i = 0; i < 10; ++i)
//        success |= bp.Search();

//    BOOST_REQUIRE_EQUAL(success, true);

//    BOOST_REQUIRE_EQUAL(bp.ExecuteBestPlan(), true);

//    std::vector<gc::GameCommandPtr> aiGcs = ai->FetchGameCommands();
//    for (gc::GameCommandPtr& gc : aiGcs)
//        gc->Execute(world, curPlayer);

//    auto blds = beowulf.GetAIInterface().GetBuildingSites();
//    BOOST_REQUIRE(std::find_if(blds.begin(), blds.end(), [](const noBuildingSite* bld) { return bld->GetBuildingType() == BLD_WELL; }) != blds.end());

//    //beowulf::CreateSvg(beowulf.GetAIInterface(), beowulf.buildings, "plan.svg");
//}

//BOOST_FIXTURE_TEST_CASE(PlanBoardProductionBuilding, BiggerWorldWithGCExecution)
//{
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

////    // Place a few trees
////    for(const MapPoint& pt : world.GetPointsInRadius(hqPos + MapPoint(4, 0), 2))
////    {
////        if(!world.GetNode(pt).obj)
////            world.SetNO(pt, new noTree(pt, 0, 3));
////    }

//    beowulf::BuildingPlanner bp(beowulf.GetAIInterface(), beowulf.buildings, beowulf.resourceMap);
//    bp.Request(BLD_SAWMILL);
//    bp.Request(BLD_WOODCUTTER);
//    bp.Request(BLD_WOODCUTTER);
//    bp.Request(BLD_FORESTER);

//    bp.Init();

//    bool success = false;
//    for (unsigned i = 0; i < 10; ++i)
//        success |= bp.Search();

//    BOOST_REQUIRE_EQUAL(success, true);

//    BOOST_REQUIRE_EQUAL(bp.ExecuteBestPlan(), true);



//    std::vector<gc::GameCommandPtr> aiGcs = ai->FetchGameCommands();
//    for (gc::GameCommandPtr& gc : aiGcs)
//        gc->Execute(world, curPlayer);

//    auto blds = beowulf.GetAIInterface().GetBuildingSites();
//    BOOST_REQUIRE(std::find_if(blds.begin(), blds.end(), [](const noBuildingSite* bld) { return bld->GetBuildingType() == BLD_SAWMILL; }) != blds.end());
//    BOOST_REQUIRE(std::find_if(blds.begin(), blds.end(), [](const noBuildingSite* bld) { return bld->GetBuildingType() == BLD_WOODCUTTER; }) != blds.end());
//    BOOST_REQUIRE(std::find_if(blds.begin(), blds.end(), [](const noBuildingSite* bld) { return bld->GetBuildingType() == BLD_FORESTER; }) != blds.end());

//    beowulf::CreateSvg(beowulf.GetAIInterface(), beowulf.buildings, "plan_boards.svg");
//}

BOOST_AUTO_TEST_SUITE_END()

#endif
