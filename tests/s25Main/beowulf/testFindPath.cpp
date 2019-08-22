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
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

#ifdef BEOWULF_ENABLE_ALL

BOOST_AUTO_TEST_SUITE(BeowulfFindPath)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(FindPathEmptyMapToHQ, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> ai(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), curPlayer, world));
    beowulf::Beowulf& beowulf = static_cast<beowulf::Beowulf&>(*ai);

    MapPoint start(8, 3);
    MapPoint dest = beowulf.world.Get(MapPoint(12, 11))->GetFlag();

    std::vector<Direction> route;
    bool found = beowulf::FindPath(start, world, &route,
    // Condition
    [&beowulf, start](const MapPoint& pt, Direction dir)
    {
        if (pt == start && dir == Direction::NORTHWEST)
            return false;

        return beowulf.world.IsRoadPossible(pt, dir);
    },
    // End
    [dest](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // island.
        return pt == dest;
    },
    // Heuristic
    [&beowulf, dest](const MapPoint& pt)
    {
        return beowulf.gwb.CalcDistance(pt, dest);
    },
    // Cost
    [](const MapPoint&, Direction)
    {
        return 1;
    });

    BOOST_REQUIRE(found);
    BOOST_REQUIRE(!route.empty());

    beowulf.world.ConstructFlag(start);
    beowulf.world.ConstructRoad(start, route);

    for (int i = 0; i < 10; ++i)
        Proceed(ai, world, curPlayer, em);

//    beowulf::AsciiMap map(beowulf.GetAIInterface(), 1);
//    map.draw(beowulf.buildings);
//    map.write();

    beowulf::FlagState state = beowulf.world.GetFlagState(start);
    BOOST_REQUIRE(state == beowulf::FlagFinished);
    BOOST_REQUIRE(beowulf.world.GetRoadState(start, route[0]) == beowulf::RoadFinished);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
