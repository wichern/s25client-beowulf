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
#include "ai/beowulf/World.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Types.h"
#include "ai/beowulf/recurrent/ExpansionPlanner.h"

#include "nodeObjs/noTree.h"
#include "nodeObjs/noFlag.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"

#include <boost/test/unit_test.hpp>

#include <memory> /* std::unique_ptr */

#include "helper.h"

BOOST_AUTO_TEST_SUITE(BeowulfExpansionPlanner)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

using beowulf::Building;
using beowulf::InvalidProductionGroup;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_FIXTURE_TEST_CASE(Simple, WorldLoaded1PFixture)
{
    std::unique_ptr<beowulf::Beowulf> beowulf(static_cast<beowulf::Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();
    beowulf->expand.Enable();

    // Wait for the expansion planner to request a new military building.
    Proceed([&]() { return !beowulf->world.GetBuildings(BLD_GUARDHOUSE).empty(); }, { beowulf.get() }, em, world);

    std::vector<Building*> guardhouses = beowulf->world.GetBuildings(BLD_GUARDHOUSE);
    BOOST_REQUIRE_EQUAL(guardhouses.size(), 1);

    Building* guardhouse = guardhouses.front();
    Proceed([&]() { return guardhouse->GetState() == beowulf::Building::UnderConstruction; }, { beowulf.get() }, em, world);

    bool success = beowulf->roads.Connect(guardhouse);
    BOOST_REQUIRE(success);

    Proceed([&]() { return guardhouse->GetState() == beowulf::Building::Finished; }, { beowulf.get() }, em, world);

    // Wait for at least three additional expansion buildings.
    Proceed([&]() { return beowulf->world.GetBuildings(BLD_GUARDHOUSE).size() > 3; }, { beowulf.get() }, em, world);

//    {
//        beowulf::AsciiMap map(beowulf->GetAIInterface(), beowulf->world.GetHQFlag(), 20);
//        map.draw(beowulf->world);
//        map.drawResources(world);
//        map.write();
//    }
}

#endif

BOOST_AUTO_TEST_SUITE_END()
