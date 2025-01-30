// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "pyBuilding.h"

#include "ai/AIResource.h"
#include "gameTypes/SettingsTypes.h"

#include <limits>
#include <memory>
#include <string>
#include <vector>

class AIPlayer;
class AIBuildLocations;

namespace s25py {

class PyGame;

// Base class for AI players
// The base class will only be a dummy (do nothing)
class PyPlayer
{
public:
    PyPlayer();
    virtual ~PyPlayer();

    std::vector<PyBuilding> GetHeadquaters() const;

    void ChangeDistribution(Distributions distributions);
    void SetConstructionSite(const MapPoint& pos, BuildingType type);
    unsigned GetResources(const MapPoint& pos, AIResource type);

protected:
    unsigned id_ = std::numeric_limits<unsigned>::max();
    PyGame* game_ = nullptr;

    // Pointer to the in-game AI player object
    AIPlayer* player_ = nullptr;

    friend class PyGame;
};

} // namespace s25py
