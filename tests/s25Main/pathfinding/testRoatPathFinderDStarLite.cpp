// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AsciiMap.h"
#include "pathfinding/RoadPathFinderDStar.h"
#include "worldFixtures/WorldWithGCExecution.h"
#include "world/MapBase.h"
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

template<RoadType T_roadType>
struct AvoidRoadType
{
    bool operator()(const RoadSegment& segment) const { return segment.GetRoadType() != T_roadType; }
};

struct CostsNone
{
    unsigned operator()(const noRoadNode&, const Direction) const { return 0; }
};

BOOST_AUTO_TEST_SUITE(Pathfinding)

BOOST_FIXTURE_TEST_CASE(GetShortestPath_NotAvailable, WorldWithGCExecution1P)
{
    this->SetBuildingSite({2, 2}, BuildingType::Farm);

    // AsciiMap debugMap(world);
    // debugMap.drawPlayer(0);
    // debugMap.write();

    // start from HQ
    nobBaseWarehouse* start = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
    BOOST_TEST_REQUIRE(start != nullptr);

    // Dest is created building.
    noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
    BOOST_TEST_REQUIRE(goal != nullptr);

    dstarlite::Search search = {world, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
                                AvoidRoadType<RoadType::Water>()};
    dstarlite::Node& startDNode = search.GetNode(*start);

    BOOST_TEST_REQUIRE(startDNode.rhs == std::numeric_limits<unsigned>::max());
    BOOST_TEST_REQUIRE(search.ComputeShortestPath(*start, 42) == false);
}

BOOST_FIXTURE_TEST_CASE(GetShortestPath_Simple, WorldWithGCExecution1P)
{
    this->SetBuildingSite({2, 2}, BuildingType::Farm);

    // start from HQ
    nobBaseWarehouse* start = world.GetSpecObj<nobBaseWarehouse>(world.GetPlayer(0).GetHQPos());
    BOOST_TEST_REQUIRE(start != nullptr);

    // Dest is created building.
    noBuildingSite* goal = world.GetSpecObj<noBuildingSite>({2, 2});
    BOOST_TEST_REQUIRE(goal != nullptr);

    // Create road between nodes
    this->BuildRoad(goal->GetFlagPos(), false,
                    {Direction::SouthEast, Direction::SouthEast, Direction::SouthEast, Direction::East, Direction::East,
                     Direction::East});

    // AsciiMap debugMap(world);
    // debugMap.drawPlayer(0);
    // debugMap.write();

    dstarlite::Search search = {world, *goal, world.GetEvMgr().GetCurrentGF(), CostsNone(),
                                AvoidRoadType<RoadType::Water>()};
    // dstarlite::Node& startDNode = search.GetNode(*start);

    BOOST_TEST_REQUIRE(search.ComputeShortestPath(*start, 42));
}

BOOST_AUTO_TEST_SUITE_END()