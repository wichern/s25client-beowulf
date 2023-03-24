// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/Types.h"
#include "ai/beowulf/ProductionConsts.h"
#include "ai/beowulf/recurrent/ProductionPlanner.h"

#include "world/MapGeometry.h"
#include "world/NodeMapBase.h"
#include "gameTypes/BuildingType.h"
#include "gameTypes/GoodTypes.h"

#include <vector>

class AIInterface;

namespace beowulf {

class Building;
class World;
class Beowulf;

// @todo: Rename
class BuildingPositionCosts
{
public:
    BuildingPositionCosts(const AIInterface& aii,
                          World& world);

    /*
     * Calculates a score vector for the given building at given point.
     * returns false if placement is invalid.
     */
    bool Score(
            std::vector<double>& score,
            const Building* building,
            const MapPoint& pt);

private:
    const AIInterface& aii_;
    World& world_;
};

} // namespace beowulf
