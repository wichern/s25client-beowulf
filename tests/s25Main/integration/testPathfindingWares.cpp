// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AsciiMap.h"
#include "factories/AIFactory.h"
#include "ai/AIPlayer.h"
#include "buildings/nobHQ.h"
#include "worldFixtures/WorldFixture.h"
#include "worldFixtures/WorldWithGCExecution.h"
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
    world.SetFlag(MapPoint(10, 13), 0);
    world.BuildRoad(0, false, MapPoint(12, 13), {Direction::NorthEast, Direction::NorthEast});
    world.BuildRoad(0, false, MapPoint(12, 13), {Direction::West, Direction::West});
    world.BuildRoad(0, false, MapPoint(10, 13), {Direction::NorthEast, Direction::NorthEast});

    // AsciiMap map(world);
    // map.drawPlayer(0);
    // map.write();

    auto& pathfinder = world.GetRoadPathFinder();

    auto* start = world.GetSpecObj<noRoadNode>(MapPoint(13, 11));
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
    nofCarrier* carrier = route->getCarrier(0);
    route->setCarrier(0, nullptr);

    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::SouthWest);

    // repeatable?
    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::SouthWest);

    // decrease cost again
    route->setCarrier(0, carrier);

    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::West);

    // completely remove edge
    start->DestroyRoad(Direction::West);

    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::SouthWest);

    // Remove flag
    world.DestroyFlag(MapPoint(10, 13), 0);

    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(!success);
}

BOOST_FIXTURE_TEST_CASE(FindPathAfterSubgraphConnection_Case1, EmptyWorldFixture1P)
{
    // Case 1: one of the flags has already been part of a search to goal
    std::unique_ptr<AIPlayer> player(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Easy), 0, world));

    world.SetFlag(MapPoint(13, 11), 0);
    world.SetFlag(MapPoint(12, 13), 0);
    world.BuildRoad(0, false, MapPoint(12, 13), {Direction::NorthEast, Direction::NorthEast});

    const auto* goal = player->getAIInterface().GetHeadquarter();
    auto& pathfinder = world.GetRoadPathFinder();

    // AsciiMap map(world);
    // map.drawPlayer(0);
    // map.drawDStar(goal);
    // map.write();

    // Make a first search so that U is already initialized
    auto* start = world.GetSpecObj<noRoadNode>(MapPoint(11, 11));
    unsigned length = 0;
    RoadPathDirection firstDir;
    MapPoint firstNodePos;
    bool success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::NorthWest);

    // Add an edge that connects the two subgraphs
    world.BuildRoad(0, false, MapPoint(13, 11), {Direction::West, Direction::West});

    start = world.GetSpecObj<noRoadNode>(MapPoint(12, 13));
    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::NorthEast);
}

#if 0
BOOST_FIXTURE_TEST_CASE(FindPathAfterSubgraphConnection_Case2, EmptyWorldFixture1P)
{
    // Case 2: none of the flags have already been part of a search to goal
    std::unique_ptr<AIPlayer> player(AIFactory::Create(AI::Info(AI::Type::Default, AI::Level::Easy), 0, world));

    world.SetFlag(MapPoint(13, 11), 0);
    world.BuildRoad(0, false, MapPoint(13, 11), {Direction::West, Direction::West});
    world.SetFlag(MapPoint(12, 13), 0);
    world.SetFlag(MapPoint(11, 15), 0);
    world.BuildRoad(0, false, MapPoint(11, 15), {Direction::NorthEast, Direction::NorthEast});

    const auto* goal = player->getAIInterface().GetHeadquarter();
    auto& pathfinder = world.GetRoadPathFinder();

    // AsciiMap map(world);
    // map.drawPlayer(0);
    // map.drawDStar(goal);
    // map.write();

    // Make a first search so that U is already initialized
    auto* start = world.GetSpecObj<noRoadNode>(MapPoint(11, 11));
    unsigned length = 0;
    RoadPathDirection firstDir;
    MapPoint firstNodePos;
    bool success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::NorthWest);

    // Add an edge that connects the two subgraphs
     world.BuildRoad(0, false, MapPoint(12, 13), {Direction::NorthEast, Direction::NorthEast});

    start = world.GetSpecObj<noRoadNode>(MapPoint(11, 15));
    success = pathfinder.FindPathForWare(
        *start, *goal, std::numeric_limits<unsigned>::max(),
        &length, &firstDir, &firstNodePos);

    BOOST_TEST_REQUIRE(success);
    BOOST_TEST_REQUIRE(toDirection(firstDir) == Direction::NorthEast);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
