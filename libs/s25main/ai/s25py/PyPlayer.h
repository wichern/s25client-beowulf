// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIResource.h"
#include "gameTypes/SettingsTypes.h"
#include "gameTypes/ChatDestination.h"
#include "gameTypes/MapCoordinates.h"

#include <limits>
#include <memory>
#include <string>
#include <vector>

class AIInterface;
class AIBuildLocations;
class nobBaseWarehouse;

namespace s25py {

class PyGame;

// Base class for AI players
class PyPlayer
{
public:
    PyPlayer();
    virtual ~PyPlayer();

    virtual void RunGF(unsigned gf, bool gfisnwf);
    virtual void OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg);

    const nobBaseWarehouse* GetHeadquater() const;

    void ChangeDistribution(Distributions distributions);
    void SetConstructionSite(const MapPoint& pos, BuildingType type);
    unsigned GetResources(const MapPoint& pos, AIResource type);

protected:
    unsigned id_ = std::numeric_limits<unsigned>::max();
    PyGame* game_ = nullptr;
    AIInterface* aii_ = nullptr;

    friend class PyGame;
    friend class AIPlayerPython;
};

// Trampoline class to support overriding virtual methods.
class PlayerTrampoline : public PyPlayer
{
public:
    using PyPlayer::PyPlayer; // Inherit the constructors
    void RunGF(unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg) override;
};

} // namespace s25py
