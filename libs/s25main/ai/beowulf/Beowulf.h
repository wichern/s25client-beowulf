// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/World.h"
#include "ai/beowulf/recurrent/AttackPlanner.h"
#include "ai/beowulf/recurrent/BuildingPlanner.h"
#include "ai/beowulf/recurrent/CoinManager.h"
#include "ai/beowulf/recurrent/ExpansionPlanner.h"
#include "ai/beowulf/recurrent/MetalworksManager.h"
#include "ai/beowulf/recurrent/ProductionPlanner.h"
#include "ai/beowulf/recurrent/RoadManager.h"

#include "ai/AIPlayer.h"

#include <ctime>
#include <vector>

namespace beowulf {

class RecurrentBase;

class Beowulf : public AIPlayer
{
public:
    World world;
    BuildingPlanner build;
    RoadManager roads;
    ExpansionPlanner expand;
    ProductionPlanner produce;
    MetalworksManager metalworks;
    AttackPlanner attack;
    CoinManager coins;

public:
    Beowulf(const unsigned char playerId, const GameWorldBase& gwb, const AI::Level level);
    ~Beowulf() override;

    void RunGF(const unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg) override;

    void DisableRecurrents();

    AIInterface& GetAII() { return aii; }
    const AIInterface& GetAII() const { return aii; }

    void ClearWorstRuntime();
    const std::vector<std::clock_t>& GetWorstRuntime() const { return recurrentsWorstRuntime_; }

private:
    void Chat(const std::string& message) const;
    bool CheckDefeat();

private:
    bool defeated_ = false;
    bool waitForNextSync_ = false;

    std::vector<std::clock_t> recurrentsWorstRuntime_;

    std::vector<RecurrentBase*> recurrents_;
};

} // namespace beowulf
