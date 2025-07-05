// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

class GameWorldBase;

namespace beowulf {

class World : public MapBase
{
public:
    World(GameWorldBase& gwb);
    virtual ~World();

private:
    GameWorldBase& gwb_;
};

} // namespace beowulf