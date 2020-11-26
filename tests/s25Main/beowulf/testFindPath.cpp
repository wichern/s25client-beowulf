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

#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

using beowulf::Beowulf;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_AUTO_TEST_SUITE(BeowulfFindPath)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(FindPathEmptyMapToHQ, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();

    MapPoint start(8, 3);
    MapPoint dest = beowulf_raw->world.GetBuilding(MapPoint(12, 11))->GetFlag();

    std::vector<Direction> route;
    bool found = beowulf::FindPath(start, world, &route,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        if (pt == start && dir == Direction::NORTHWEST)
            return false;

        return beowulf_raw->world.IsRoadPossible(pt, dir, false);
    },
    // End
    [dest](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // island.
        return pt == dest;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return beowulf->gwb.CalcDistance(pt, dest);
    },
    // Cost
    [](const MapPoint&, Direction)
    {
        return 1;
    });

    BOOST_REQUIRE(found);
    BOOST_REQUIRE(!route.empty());

    beowulf_raw->world.ConstructFlag(start);
    beowulf_raw->world.ConstructRoad(start, route);

    Proceed({ beowulf.get() }, em, world);


    beowulf::FlagState state = beowulf_raw->world.GetFlagState(start);
    BOOST_REQUIRE(state == beowulf::FlagFinished);
    BOOST_REQUIRE(beowulf_raw->world.GetRoadState(start, route[0]) == beowulf::RoadFinished);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
