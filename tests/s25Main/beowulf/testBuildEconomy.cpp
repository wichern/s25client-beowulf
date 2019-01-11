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

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"
#include "ai/aijh/AIPlayerJH.h"
#include "factories/AIFactory.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfEconomy)

#ifndef DISABLE_ALL_BEOWULF_TESTS

using beowulf::Beowulf;

BOOST_FIXTURE_TEST_CASE(Bergschlumpf, WorldLoaded1PFixture)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));

    unsigned totalGf;
    bool ret = Proceed([&]() {
        auto metalworks = beowulf->world.GetBuildings(BLD_METALWORKS);

        if (metalworks.empty())
            return false;

        return metalworks.front()->GetState() == beowulf::Building::Finished;
    }, { beowulf.get() }, em, world, 30000, &totalGf);

//    beowulf::AsciiMap map(beowulf->GetAII());
//    map.drawResources();
//    map.draw(beowulf.get());
//    map.drawBorder(beowulf->world, false);
//    map.write();

    // Smoke tests.
    BOOST_REQUIRE(ret);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
