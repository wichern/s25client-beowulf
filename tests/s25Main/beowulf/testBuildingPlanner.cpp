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

#include "factories/AIFactory.h"
#include "buildings/noBuildingSite.h"

#include <boost/test/unit_test.hpp>

#include "helper.h"

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_AUTO_TEST_SUITE(BeowulfBuildingPlanner)

//BOOST_FIXTURE_TEST_CASE(FindPath, BiggerWorldWithGCExecution)
//{
//    /*
//     * We create a world in which the destination can be one of two flags
//     * and we expect the algorithm to find the closest.
//     */

//    AI::Info ai_info(AI::BEOWULF, AI::HARD);
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
//    beowulf::Buildings& buildings = beowulf.buildings;

//    MapPoint hqFlag = buildings.Get(MapPoint(12, 11))->GetFlag();

//    buildings.ConstructFlag(MapPoint(hqFlag.x - 5, hqFlag.y));
//    buildings.ConstructRoad(hqFlag, { Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST, Direction::WEST });

//    buildings.ConstructFlag(MapPoint(hqFlag.x + 3, hqFlag.y));
//    buildings.ConstructRoad(hqFlag, { Direction::EAST, Direction::EAST, Direction::EAST });

//    for (int i = 0; i < 20; ++i) {
//        Proceed(ai, world, curPlayer, em);
//    }

//    beowulf::rnet_id_t rnet = buildings.GetRoadNetwork(hqFlag);
//    MapPoint start(12, 16);

//    std::vector<Direction> route;
//    bool found = beowulf::FindPath(start, world,
//                                   // Condition
//                                   [buildings, start](const MapPoint& pos, Direction dir)
//    {
//        if (pos == start && dir == Direction::NORTHWEST)
//            return false;

//        return buildings.IsRoadPossible(pos, dir);
//    },
//    // Cost
//    [](const MapPoint& pos, Direction dir)
//    {
//        (void)pos;
//        (void)dir;
//        return 1; // @todo: include bq degradation.
//    },
//    // End
//    [&](const MapPoint& pos)
//    {
//        // The search can end if we found a way to any flag of the destination
//        // island.
//        return buildings.GetRoadNetwork(pos) == rnet;
//    }, &route);


//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.draw(start, "x");
//    map.write();

//    BOOST_REQUIRE(found);
//}

//BOOST_FIXTURE_TEST_CASE(PlanSingleBuilding, BiggerWorldWithGCExecution)
//{
//    AI::Info ai_info(AI::BEOWULF, AI::HARD);
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
//    beowulf::Buildings& buildings = beowulf.buildings;

//    beowulf::Building* bld = buildings.Create(BLD_BREWERY, beowulf::Building::PlanningRequest);
//    beowulf.RequestConstruction(bld, buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag()));

//    while (bld->GetState() != beowulf::Building::UnderConstruction) {
//        Proceed(ai, world, curPlayer, em);
//    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.write();
//}

//BOOST_FIXTURE_TEST_CASE(PlanMultipleBuildings, BiggerWorldWithGCExecution)
//{
//    AI::Info ai_info(AI::BEOWULF, AI::HARD);
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
//    beowulf::Buildings& buildings = beowulf.buildings;

//    std::vector<beowulf::Building*> requests;
//    requests.push_back(buildings.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_FORESTER, beowulf::Building::PlanningRequest));

//    beowulf::rnet_id_t island = buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag());
//    for (beowulf::Building* bld : requests)
//        beowulf.RequestConstruction(bld, island);

//    while (requests.front()->GetState() != beowulf::Building::UnderConstruction) {
//        Proceed(ai, world, curPlayer, em);
//    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.drawResources(ai->gwb);
//    map.draw(beowulf.buildings);
//    map.write();
//}

//BOOST_FIXTURE_TEST_CASE(PlanManyBuildings, BiggerWorldWithGCExecution)
//{
//    AI::Info ai_info(AI::BEOWULF, AI::HARD);
//    std::unique_ptr<AIPlayer> ai(AIFactory::Create(ai_info, curPlayer, world));
//    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);
//    beowulf::Buildings& buildings = beowulf.buildings;

//    std::vector<beowulf::Building*> requests;
//    requests.push_back(buildings.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_WOODCUTTER, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_FORESTER, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_STOREHOUSE, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_QUARRY, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_QUARRY, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_MILL, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_BAKERY, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_WELL, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));
//    requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));

//    beowulf::rnet_id_t island = buildings.GetRoadNetwork(buildings.Get(MapPoint(12, 11))->GetFlag());
//    for (beowulf::Building* bld : requests)
//        beowulf.RequestConstruction(bld, island);

//    while (requests.front()->GetState() != beowulf::Building::UnderConstruction) {
//        Proceed(ai, world, curPlayer, em);
//    }

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.write();
//}

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
