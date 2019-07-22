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

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/BuildingQualityCalculator.h"

#include "nodeObjs/noTree.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfBuildLocations)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(BuildLocationsEmptyMap, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    const beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    beowulf::BuildingQualityCalculator bqc(beowulf.GetAIInterface());
    beowulf::BuildLocations bl(beowulf.GetAIInterface().gwb);

    {
        bl.Calculate(world, bqc, &beowulf.buildings, MapPoint(13, 12)); // HQ flag

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Calling Calculate() a second time.
    {
        bl.Calculate(world, bqc, &beowulf.buildings, MapPoint(13, 12)); // HQ flag

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 181);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 190);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 561);
    }

    // Create a territory with a non reachable section:
    {
        world.SetNO({13, 3}, new noTree({13, 3}, 0, 3));
        world.SetNO({14, 4}, new noTree({14, 4}, 0, 3));
        world.SetNO({14, 5}, new noTree({14, 5}, 0, 3));
        world.SetNO({15, 6}, new noTree({15, 6}, 0, 3));
        world.SetNO({15, 7}, new noTree({15, 7}, 0, 3));
        world.SetNO({16, 7}, new noTree({16, 7}, 0, 3));
        world.SetNO({17, 7}, new noTree({17, 7}, 0, 3));
        world.SetNO({18, 7}, new noTree({18, 7}, 0, 3));

        bl.Update(bqc, {13, 3}, 2);
        bl.Update(bqc, {14, 4}, 2);
        bl.Update(bqc, {14, 5}, 2);
        bl.Update(bqc, {15, 6}, 2);
        bl.Update(bqc, {15, 7}, 2);
        bl.Update(bqc, {16, 7}, 2);
        bl.Update(bqc, {17, 7}, 2);
        bl.Update(bqc, {18, 7}, 2);

        // no duplicates:
        std::set<unsigned> set;
        for (const MapPoint& pt : bl.Get(BQ_HUT)) {
            unsigned idx = world.GetIdx(pt);
            BOOST_REQUIRE(set.find(idx) == set.end());
            set.insert(idx);
        }

        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HUT).size(), bl.GetSize());
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_CASTLE).size(), 157);
        BOOST_REQUIRE_EQUAL(bl.Get(BQ_HOUSE).size(), 166);
        BOOST_REQUIRE_EQUAL(bl.GetSum(), 502);
    }
}

BOOST_AUTO_TEST_SUITE_END()
