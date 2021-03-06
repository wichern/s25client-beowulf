// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef BEOWULF_HEURISTICS_H_INCLUDED
#define BEOWULF_HEURISTICS_H_INCLUDED

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

#endif //! BEOWULF_HEURISTICS_H_INCLUDED
