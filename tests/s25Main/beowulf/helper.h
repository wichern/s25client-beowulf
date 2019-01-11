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

#pragma once

#include "rttrDefines.h" // IWYU pragma: keep
#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Types.h"

#include "nodeObjs/noFlag.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"

#include <memory> /* std::unique_ptr */

void Proceed(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world,
        unsigned& player,
        TestEventManager& em);

bool ConstructBuilding(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world,
        unsigned& player,
        TestEventManager& em,
        BuildingType type,
        const MapPoint& pos,
        bool wait_for_site);

bool CompareBuildingsWithWorld(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world);
