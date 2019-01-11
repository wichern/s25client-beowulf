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

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"
#include "ai/aijh/AIPlayerJH.h"
#include "ai/DummyAI.h"
#include "factories/AIFactory.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

BOOST_AUTO_TEST_SUITE(Beowulf1vs1)

#ifdef ENABLE_BEOWULF_FIGHTS

using beowulf::Beowulf;
using AIJH::AIPlayerJH;

char MapName[] = "Bergstrasse.swd";
char ReplayName[] = "replay.rpl";
typedef WorldAIBattle<MapName, 3, ReplayName> WorldBergschlumpf2PL;

BOOST_FIXTURE_TEST_CASE(Bergschlumpf_1vs1, WorldBergschlumpf2PL)
{
    std::shared_ptr<DummyAI> dummy = CreatePlayer<DummyAI>(AI::DUMMY);
    std::shared_ptr<Beowulf> beowulf = CreatePlayer<Beowulf>(AI::BEOWULF);
    std::shared_ptr<AIPlayerJH> aijh = CreatePlayer<AIPlayerJH>(AI::DEFAULT);

    beowulf::AsciiTable table(8);
    table.addRow({ "GF", "build", "roads", "expand", "produce", "metalworks", "attack", "coins" });

    for (unsigned i = 0; i < 100; ++i) {
        unsigned totalGf;
        bool ret = Proceed([&]()
        {
            return aijh->player.IsDefeated() || beowulf->player.IsDefeated();
        }, 1000, &totalGf);

        DrawAsciiMap();

        std::vector<std::string> row;
        row.push_back(std::to_string(em.GetCurrentGF()));
        for (std::clock_t clocks : beowulf->GetWorstRuntime()) {
            double ms = static_cast<double>(clocks) * 1000.0 / static_cast<double>(CLOCKS_PER_SEC);
            row.push_back(std::to_string(ms));
        }
        table.addRow(row);
        std::cout << "Performance" << std::endl;
        table.write();
        beowulf->ClearWorstRuntime();

        std::cout << "Stats" << std::endl;
        DrawAllPlayerStatistics();

        if (ret)
            break;
    }

    if (beowulf->player.IsDefeated())
        std::cout << "Beowulf lost" << std::endl;
    if (aijh->player.IsDefeated())
        std::cout << "AIJH lost" << std::endl;
}

#endif

BOOST_AUTO_TEST_SUITE_END()
