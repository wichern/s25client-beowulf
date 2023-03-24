// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/recurrent/RecurrentBase.h"

namespace beowulf {

class Building;

class CoinManager : public RecurrentBase
{
public:
    CoinManager(Beowulf* beowulf);

    void OnRun() override;

    void OnBuildingNote(const BuildingNote& note) override;

private:
    Building* academy_ = nullptr;

    void RequestAcademy();
};

} // namespace beowulf
