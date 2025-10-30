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


BOOST_AUTO_TEST_SUITE_END()