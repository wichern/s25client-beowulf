// Copyright (c) 2005 - 2019 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef BEOWULF_RECURRENT_ATTACKPLANNER_H_INCLUDED
#define BEOWULF_RECURRENT_ATTACKPLANNER_H_INCLUDED

#include "ai/beowulf/recurrent/RecurrentBase.h"

#include "gameTypes/MapCoordinates.h"

class nobBaseMilitary;

namespace beowulf {

class AttackPlanner : public RecurrentBase
{
public:
    AttackPlanner(Beowulf* beowulf);

    void OnRun() override;

private:
    // Filters targets that are already under attack
    std::vector<const nobBaseMilitary*> GetPotentialTargets() const;
    std::array<unsigned, 5> GetAvailableAttackers(const MapPoint& pt) const;
    unsigned GetAttackersCount(const std::array<unsigned, 5>& soldiers, unsigned char enemy) const;
};

} // namespace beowulf

#endif //! BEOWULF_RECURRENT_ATTACKPLANNER_H_INCLUDED
