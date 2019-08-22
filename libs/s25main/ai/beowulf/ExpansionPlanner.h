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
#ifndef BEOWULF_EXPANSIONPLANNER_H_INCLUDED
#define BEOWULF_EXPANSIONPLANNER_H_INCLUDED

#include "ai/beowulf/Types.h"

#include <vector>

class AIInterface;

namespace beowulf {

class World;
class Building;

class ExpansionPlanner
{
public:
    ExpansionPlanner(AIInterface& aii, World& world);

    void Update(rnet_id_t rnet, std::vector<Building*>& requests);

private:
    bool ShouldExpand() const;

    AIInterface& aii_;
    World& world_;

    const unsigned checkInterval_ = 100;
    const unsigned minSoldiers_ = 5;
    const unsigned maxParallelSites_ = 3;

    unsigned lastCheck_ = checkInterval_;
};

} // namespace beowulf

#endif //! BEOWULF_EXPANSIONPLANNER_H_INCLUDED
