// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

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
