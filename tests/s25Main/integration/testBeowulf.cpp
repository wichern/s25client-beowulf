// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/RoadBuilder.h"
#include "ai/beowulf/Beowulf.h"
#include "AsciiMap.h"
#include "factories/AIFactory.h"
#include "buildings/nobHQ.h"
#include "nodeObjs/noTree.h"
#include "worldFixtures/WorldWithGCExecution.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(BeowulfBuildLocations)

using BiggerWorldWithGCExecution = WorldWithGCExecution<1, 24, 22>;

BOOST_FIXTURE_TEST_CASE(EmptyMap, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::Type::Beowulf, AI::Level::Hard), 0, world));

    // Counting found locations on an empty map.
    {
        beowulf::BuildLocations bl(beowulf->getAIInterface());
        bl.Calculate(beowulf->getAIInterface().GetHeadquarter()->GetFlagPos());
        // AsciiMap map(beowulf->getAIInterface().gwb);
        // map.drawPlayer(beowulf->GetPlayerId());
        // for (auto const& [pt, bq] : bl.GetAll())
        //     map.drawBq(pt, bq);
        // map.write();

        // Check for duplicates
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BuildingQuality::Hut)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Hut).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Castle).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::House).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Calling Calculate() a second time should lead to the same results.
    {
        beowulf::BuildLocations bl(beowulf->getAIInterface());
        bl.Calculate(beowulf->getAIInterface().GetHeadquarter()->GetFlagPos());

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BuildingQuality::Hut)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Hut).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Castle).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::House).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Create a territory with a non reachable section.
    // Non reachable build locations should not be returned by BuildLocations.
    {
        world.SetNO({13, 3}, new noTree({13, 3}, 0, 3));
        world.SetNO({14, 4}, new noTree({14, 4}, 0, 3));
        world.SetNO({14, 5}, new noTree({14, 5}, 0, 3));
        world.SetNO({15, 6}, new noTree({15, 6}, 0, 3));
        world.SetNO({15, 7}, new noTree({15, 7}, 0, 3));
        world.SetNO({16, 7}, new noTree({16, 7}, 0, 3));
        world.SetNO({17, 7}, new noTree({17, 7}, 0, 3));
        world.SetNO({18, 7}, new noTree({18, 7}, 0, 3));

        world.InitAfterLoad();

        beowulf::BuildLocations bl(beowulf->getAIInterface());
        bl.Calculate(beowulf->getAIInterface().GetHeadquarter()->GetFlagPos());

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BuildingQuality::Hut)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        // AsciiMap map(world);
        // map.drawPlayer(beowulf->GetPlayerId());
        // for (auto const& [pt, bq] : bl.GetAll())
        //     map.drawBq(pt, bq);
        // map.write();

        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Hut).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Castle).size(), 151);
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::House).size(), 160);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 481);
    }
}

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(BeowulfRoadBuilder)

using BiggerWorldWithGCExecution = WorldWithGCExecution<1, 24, 22>;

BOOST_FIXTURE_TEST_CASE(EmptyMap, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::Type::Beowulf, AI::Level::Hard), 0, world));


    MapPoint hqFlag = beowulf->getAIInterface().GetHeadquarter()->GetFlagPos();

    beowulf::RoadBuilder builder(beowulf->getAIInterface());
    
    // Can connect flag to itself
    BOOST_REQUIRE(builder.CanConnect(hqFlag, hqFlag));
    
    // Can connect two flags
    world.SetFlag({17, 12}, beowulf->GetPlayerId());
    BOOST_REQUIRE(builder.CanConnect({15, 12}, hqFlag));
    
    // Can connect two non existing points
    BOOST_REQUIRE(builder.CanConnect({16, 12}, {10, 14}));

    AsciiMap map(beowulf->getAIInterface().gwb);
    map.drawPlayer(beowulf->GetPlayerId());
    map.write();

    std::vector<Direction> route;
    BOOST_REQUIRE(builder.FindConnectionToNearestFlag({17, 12}, &route, beowulf::RoadBuilder::ShortestNoFlags));
    world.BuildRoad(beowulf->GetPlayerId(), false, {17, 12}, route);
    
    map.clear();
    map.drawPlayer(beowulf->GetPlayerId());
    map.write();

    std::vector<MapPoint> flags;
    world.SetFlag({20, 12}, beowulf->GetPlayerId());
    BOOST_REQUIRE(builder.FindConnectionToNearestFlag({20, 12}, &route, beowulf::RoadBuilder::TwoSegmentFlags, &flags));
    world.BuildRoad(beowulf->GetPlayerId(), false, {20, 12}, route);
    for (const auto& flag : flags)
        world.SetFlag(flag, beowulf->GetPlayerId());
    
    map.clear();
    map.drawPlayer(beowulf->GetPlayerId());
    map.write();

    world.SetFlag({14, 13}, beowulf->GetPlayerId());
    BOOST_REQUIRE(builder.FindConnectionToNearestFlag({14, 13}, &route, beowulf::RoadBuilder::ShortestNoFlags));
    world.BuildRoad(beowulf->GetPlayerId(), false, {14, 13}, route);
    
    map.clear();
    map.drawPlayer(beowulf->GetPlayerId());
    map.write();

    // Using anticipated buildings at 19,9
    beowulf::RoadBuilder builder_anticipated(beowulf->getAIInterface(), {19, 9}, BuildingQuality::House);
    BOOST_REQUIRE(builder_anticipated.FindConnectionToNearestFlag({20, 10}, &route, beowulf::RoadBuilder::ShortestNoFlags));
    world.SetFlag({20, 10}, beowulf->GetPlayerId());
    world.BuildRoad(beowulf->GetPlayerId(), false, {20, 10}, route);
    
    map.clear();
    map.drawPlayer(beowulf->GetPlayerId());
    map.write();
}

BOOST_AUTO_TEST_SUITE_END()
