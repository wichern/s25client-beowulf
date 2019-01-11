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
#ifndef BEOWULF_PRODUCTIONPLANNER_H_INCLUDED
#define BEOWULF_PRODUCTIONPLANNER_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "gameTypes/BuildingType.h"

#include <string>

class AIInterface;

namespace beowulf {

class Beowulf;
class Map;
class BuildingPlanner;

/**
 * @brief Planning Beowulfs production.
 *
 * The heart of Beowulf is his ProductionPlanner. It evaluates the needed amount
 * of resources and requests the required buildings.
 */
class ProductionPlanner
{
public:
    ProductionPlanner(Beowulf& beowulf);
    ~ProductionPlanner();

    bool Plan();

    void Debug(const std::string& path) const;

    unsigned GetGoal(BGoodType type) const { return goal_[type]; }

private:
    /// Request an increase in production of the given type.
    void IncreaseProductionGoal(BGoodType type);

    /// Add a suitable producer for the given good type.
    /// This will lead to building requests for the whole production chain
    void ExtendProduction(BGoodType type);

    /// Update production_ and required_ after a new buiding of given type would
    /// exist.
    void AddBuilding(BuildingType type);

    Beowulf& beowulf_;

    // desired production values
    unsigned goal_[BGD_COUNT];

    // current production (temp)
    unsigned production_[BGD_COUNT];

    // required production (temp)
    unsigned required_[BGD_COUNT];
};

} // namespace beowulf

#endif //! BEOWULF_PRODUCTIONPLANNER_H_INCLUDED
