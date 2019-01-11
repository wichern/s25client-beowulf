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
#ifndef BEOWULF_COSTFUNCTIONS_H_INCLUDED
#define BEOWULF_COSTFUNCTIONS_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/MapGeometry.h"
#include "world/NodeMapBase.h"
#include "gameTypes/BuildingType.h"
#include "gameTypes/GoodTypes.h"

#include <vector>

class AIInterface;

namespace beowulf {

class Building;
class BuildingsBase;
class BuildingQualityCalculator;
class ResourceMap;

// @todo: Rename to Score
class BuildingPositionCosts
{
public:
    BuildingPositionCosts(const AIInterface& aii,
                          BuildingQualityCalculator& bqc,
                          ResourceMap& resourceMap,
                          const BuildingsBase* buildings);

    /*
     * Calculates a score vector for the given building at given point.
     * returns false if placement is invalid.
     */
    bool Score(
            std::vector<double>& score,
            const Building* building,
            const MapPoint& pt,
            Island island);

private:
    const AIInterface& aii_;
    BuildingQualityCalculator& bqc_;
    ResourceMap& resourceMap_;
    const BuildingsBase* buildings_;
};

} // namespace beowulf

#endif //! BEOWULF_COSTFUNCTIONS_H_INCLUDED
