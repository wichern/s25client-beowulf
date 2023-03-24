// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/recurrent/RecurrentBase.h"
#include "ai/beowulf/Types.h"

#include "gameTypes/BuildingQuality.h"

#include <vector>

class AIInterface;

namespace beowulf {

class World;
class Building;

class ExpansionPlanner : public RecurrentBase
{
public:
    ExpansionPlanner(Beowulf* beowulf);

    void OnRun() override;

private:
    void Expand(const MapPoint& pt);
    bool ShouldExpand() const;
    bool TryImprove(BuildingType& type, BuildingQuality bq) const;

    const unsigned minSoldiers_ = 5;
    const unsigned maxParallelSites_ = 3;
};

} // namespace beowulf
