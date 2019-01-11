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
#include "ai/beowulf/Resources.h"

#include "helper.h"

using beowulf::Beowulf;

#include <boost/test/unit_test.hpp>

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_AUTO_TEST_SUITE(BeowulfRoadManager)

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_FIXTURE_TEST_CASE(Connect, BiggerWorldWithGCExecution)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
}

#endif

BOOST_AUTO_TEST_SUITE_END()
