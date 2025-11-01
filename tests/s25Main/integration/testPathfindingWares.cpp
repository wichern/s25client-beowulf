// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AsciiMap.h"
#include "factories/AIFactory.h"
#include "ai/AIPlayer.h"
#include "buildings/nobHQ.h"
#include "worldFixtures/WorldFixture.h"
#include "worldFixtures/CreateEmptyWorld.h"
#include "pathfinding/RoadPathFinder.h"
#include "gameTypes/GameTypesOutput.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PathfindingWaresSuite)

using EmptyWorldFixture1P = WorldFixture<CreateEmptyWorld, 1, 22, 20>;

BOOST_FIXTURE_TEST_CASE(GoalEqualsStart, EmptyWorldFixture1P)
{
    std::unique_ptr<AIPlayer> player(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Easy), 0, world));
    auto& pathfinder = world.GetRoadPathFinder();

    unsigned length = std::numeric_limits<unsigned>::max();
    bool success = pathfinder.FindPathForWare(
        *player->getAIInterface().GetHeadquarter(),
        *player->getAIInterface().GetHeadquarter(),
        std::numeric_limits<unsigned>::max(),
        &length);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(length == 0);
}

BOOST_FIXTURE_TEST_CASE(FindPathOneSegment, EmptyWorldFixture1P)
{
    std::unique_ptr<AIPlayer> player(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Easy), 0, world));

    world.SetFlag(MapPoint(13, 11), 0);
    world.BuildRoad(0, false, MapPoint(13, 11), {Direction::West, Direction::West});

    // AsciiMap map(world);
    // map.drawPlayer(0);
    // map.write();

    auto& pathfinder = world.GetRoadPathFinder();

    const auto* start = world.GetSpecObj<noRoadNode>(MapPoint(13, 11));
    const auto* goal = player->getAIInterface().GetHeadquarter()->GetFlag();

    unsigned length = 0;
    RoadPathDirection firstDir;
    MapPoint firstNodePos;
    bool success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
}

BOOST_FIXTURE_TEST_CASE(FindPathTriangle, EmptyWorldFixture1P)
{
    std::unique_ptr<AIPlayer> player(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Easy), 0, world));

    world.SetFlag(MapPoint(13, 11), 0);
    world.BuildRoad(0, false, MapPoint(13, 11), {Direction::West, Direction::West});
    world.SetFlag(MapPoint(12, 13), 0);
    world.BuildRoad(0, false, MapPoint(12, 13), {Direction::NorthEast, Direction::NorthEast});
    world.BuildRoad(0, false, MapPoint(12, 13), {Direction::NorthWest, Direction::NorthWest});

    // AsciiMap map(world);
    // map.drawPlayer(0);
    // map.write();

    auto& pathfinder = world.GetRoadPathFinder();

    const auto* start = world.GetSpecObj<noRoadNode>(MapPoint(13, 11));
    const auto* goal = player->getAIInterface().GetHeadquarter()->GetFlag();

    unsigned length = 0;
    RoadPathDirection firstDir;
    MapPoint firstNodePos;
    bool success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::West);

    // Change edge cost so that we go the other way
    RoadSegment* route = start->GetRoute(Direction::West);
    route->setCarrier(0, nullptr);
    
    // mark nodes as dirty (@todo: use a mark if exists (we can keep the iterator from Exists()))
    if (route->GetF1()->dstar.Exists(*goal))
        pathfinder.OnEdgeCostChanged(*route->GetF1());
    if (route->GetF1() != route->GetF2() && route->GetF2()->dstar.Exists(*goal))
        pathfinder.OnEdgeCostChanged(*route->GetF2());


    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::SouthWest);
}

BOOST_AUTO_TEST_SUITE_END()
