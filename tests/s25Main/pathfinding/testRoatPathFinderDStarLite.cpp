// // Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
// //
// // SPDX-License-Identifier: GPL-2.0-or-later

// #include "AsciiMap.h"
// #include "pathfinding/RoadPathFinderDStar.h"
// #include "worldFixtures/WorldWithGCExecution.h"
// #include "world/MapBase.h"
// #include <boost/test/unit_test.hpp>
// #include "gameTypes/GameTypesOutput.h"
// #include "PointOutput.h"

// #include <fstream>
// #include <iostream>

// template<RoadType T_roadType>
// struct AvoidRoadType
// {
//     bool operator()(const RoadSegment& segment) const { return segment.GetRoadType() != T_roadType; }
// };

// struct CostsNone
// {
//     unsigned operator()(const noRoadNode&, const Direction) const { return 0; }
// };

// BOOST_AUTO_TEST_SUITE(Pathfinding)

// BOOST_FIXTURE_TEST_CASE(GetShortestPath_NotAvailable, WorldWithGCExecution1P)
// {
//     this->SetBuildingSite({2, 2}, BuildingType::Farm);

//     // start from HQ
//     nobBaseWarehouse* start = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
//     BOOST_TEST_REQUIRE(start != nullptr);

//     // Dest is created building.
//     noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
//     BOOST_TEST_REQUIRE(goal != nullptr);

//     dstarlite::Search search = {world, *start, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
//                                 AvoidRoadType<RoadType::Water>()};
//     dstarlite::Node& startDNode = search.GetNode(*start);

//     BOOST_TEST_REQUIRE(startDNode.rhs == std::numeric_limits<unsigned>::max());
//     BOOST_TEST_REQUIRE(search.ComputeShortestPath(42) == false);
// }

// BOOST_FIXTURE_TEST_CASE(GetShortestPath_Simple, WorldWithGCExecution1P)
// {
//     this->SetBuildingSite({2, 2}, BuildingType::Farm);

//     // start from HQ
//     nobBaseWarehouse* start = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
//     BOOST_TEST_REQUIRE(start != nullptr);

//     // Dest is created building.
//     noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
//     BOOST_TEST_REQUIRE(goal != nullptr);

//     // Create road between nodes
//     this->BuildRoad(goal->GetFlagPos(), false,
//                     {Direction::SouthEast, Direction::SouthEast, Direction::SouthEast, Direction::East,
//                     Direction::East,
//                      Direction::East});

//     dstarlite::Search search = {world, *start, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
//                                 AvoidRoadType<RoadType::Water>()};

//     BOOST_TEST_REQUIRE(search.ComputeShortestPath(42));
// }

// BOOST_FIXTURE_TEST_CASE(GetShortestPath_AlternativeRoutes, WorldWithGCExecution1P)
// {
//     this->SetBuildingSite({2, 2}, BuildingType::Farm);

//     // start from HQ
//     nobBaseWarehouse* start = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
//     BOOST_TEST_REQUIRE(start != nullptr);

//     // Dest is created building.
//     noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
//     BOOST_TEST_REQUIRE(goal != nullptr);

//     // Create roads between nodes
//     this->BuildRoad(goal->GetFlagPos(), false,
//                     {Direction::SouthEast, Direction::SouthEast, Direction::SouthEast, Direction::East,
//                     Direction::East,
//                      Direction::East});
//     this->BuildRoad(goal->GetFlagPos(), false,
//                     {Direction::East, Direction::East, Direction::East, Direction::East, Direction::East,
//                     Direction::SouthEast, Direction::SouthEast, Direction::SouthWest, Direction::West});
//     this->SetFlag(MapPoint(3, 5));
//     this->SetFlag(MapPoint(5, 6));
//     this->SetFlag(MapPoint(7, 3));

//     dstarlite::Search search = {world, *start, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
//                                 AvoidRoadType<RoadType::Water>()};

//     BOOST_TEST_REQUIRE(search.ComputeShortestPath(42));
// }

// BOOST_FIXTURE_TEST_CASE(GetShortestPath_ReusePreviousSearchResults, WorldWithGCExecution1P)
// {
//     this->SetBuildingSite({2, 2}, BuildingType::Farm);
//     this->SetBuildingSite({10, 2}, BuildingType::Farm);

//     // start from HQ
//     nobBaseWarehouse* start1 = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
//     BOOST_TEST_REQUIRE(start1 != nullptr);
//     noBuildingSite* start2 = world.GetSpecObj<noBuildingSite>(MapPoint(10, 2));
//     BOOST_TEST_REQUIRE(start2 != nullptr);

//     // Dest is created building.
//     noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
//     BOOST_TEST_REQUIRE(goal != nullptr);

//     // Create roads between nodes
//     this->BuildRoad(goal->GetFlagPos(), false,
//                     {Direction::SouthEast, Direction::SouthEast, Direction::SouthEast, Direction::East,
//                     Direction::East,
//                      Direction::East});
//     this->BuildRoad(goal->GetFlagPos(), false,
//                     {Direction::East, Direction::East, Direction::East, Direction::East, Direction::East,
//                     Direction::SouthEast, Direction::SouthEast, Direction::SouthWest, Direction::West});
//     this->BuildRoad(start2->GetFlagPos(), false,
//                     {Direction::West, Direction::West, Direction::West});
//     this->SetFlag(MapPoint(3, 5));
//     this->SetFlag(MapPoint(5, 6));
//     this->SetFlag(MapPoint(7, 3));

//     dstarlite::Search search1 = {world, *start1, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
//                                 AvoidRoadType<RoadType::Water>()};

//     const noRoadNode* nextNode = nullptr;
//     Direction nextDir = Direction::East;
//     bool hasNextDir = search1.GetNextDirection(42, &nextNode, &nextDir);
//     BOOST_TEST_REQUIRE(hasNextDir);
//     BOOST_TEST_REQUIRE(nextNode);
//     BOOST_TEST_REQUIRE(nextNode->GetPos() == MapPoint(7, 6));
//     BOOST_TEST_REQUIRE(nextDir == Direction::SouthEast);

//     std::cout << "\nsecond search starts here\n\n";

//     dstarlite::Search search2 = {world, *start2, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
//                                 AvoidRoadType<RoadType::Water>()};
//     hasNextDir = search2.GetNextDirection(42, &nextNode, &nextDir);
//     BOOST_TEST_REQUIRE(hasNextDir);
// }

// BOOST_AUTO_TEST_SUITE_END()