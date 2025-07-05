// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildLocations.h"
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

    beowulf::BuildLocations bl(beowulf->getAIInterface());

    // Counting found locations on an empty map.
    {
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

        bl.Calculate(beowulf->getAIInterface().GetHeadquarter()->GetFlagPos());

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BuildingQuality::Hut)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        // AsciiMap map(beowulf->getAIInterface().gwb);
        // map.drawPlayer(beowulf->GetPlayerId());
        // for (auto const& [pt, bq] : bl.GetAll())
        //     map.drawBq(pt, bq);
        // map.write();

        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Hut).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::Castle).size(), 151);
        BOOST_REQUIRE_EQUAL(bl.Get(BuildingQuality::House).size(), 160);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 481);
    }

    BOOST_TEST_REQUIRE(true);
}

BOOST_AUTO_TEST_SUITE_END()
