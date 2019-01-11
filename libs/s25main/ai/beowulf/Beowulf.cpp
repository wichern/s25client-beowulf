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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/recurrent/ProductionPlanner.h"

#include "network/GameClient.h"
#include "network/GameMessages.h"

#include <cstdio>

namespace beowulf {

enum Event {
    PlaceBuildingsEvent = 0,
    PlanRoadsEvent,
    UpdateResoucesForProductinoPlanningEvent,
    PlanProductionEvent,
};

Beowulf::Beowulf(const unsigned char playerId,
                 const GameWorldBase& gwb,
                 const AI::Level level)
    : AIPlayer(playerId, gwb, level),
      world(this, false),
      build(this),
      roads(this),
      expand(this),
      produce(this),
      metalworks(this),
      attack(this),
      coins(this)
{
    recurrents_.push_back(&build);
    recurrents_.push_back(&roads);
    recurrents_.push_back(&expand);
    recurrents_.push_back(&produce);
    recurrents_.push_back(&metalworks);
    recurrents_.push_back(&attack);
    recurrents_.push_back(&coins);

    recurrentsWorstRuntime_.resize(recurrents_.size());
    ClearWorstRuntime();
}

Beowulf::~Beowulf()
{
}

void Beowulf::RunGF(const unsigned gf, bool gfisnwf)
{
    if (CheckDefeat())
        return;

    if (waitForNextSync_ && gfisnwf)
        waitForNextSync_ = false;

    // Wait for next synchronization frame and only do calculations every 16th game frame.
    if (waitForNextSync_ || (static_cast<unsigned char>(gf) & 0xF) != playerId)
        return;

    for (size_t i = 0; i < recurrents_.size(); ++i) {
        std::clock_t start = std::clock();
        recurrents_[i]->RunGf();
        std::clock_t end = std::clock();
        recurrentsWorstRuntime_[i] = std::max(recurrentsWorstRuntime_[i], end - start);
    }

    if (aii.HasIssuedGameCommands())
        waitForNextSync_ = true;

    // ResolveGoodsJams();
    // PlaceAdditionalFlags();
    // ConnectRoadNetworks();
    // PlanProduction
    // PlanStorehouses
    // DistributeGoods
    // PlanExpansion();
    // PlanAttack
    // PlanTrading
    // PlanAlliances
    // Remove trees above minerals.
    // DecommissionUnusedRoads();
}

void Beowulf::DisableRecurrents()
{
    for (RecurrentBase* recurrent : recurrents_)
        recurrent->Disable();
}

void Beowulf::ClearWorstRuntime()
{
    std::fill(recurrentsWorstRuntime_.begin(), recurrentsWorstRuntime_.end(), 0);
}

bool Beowulf::CheckDefeat()
{
    // Check for defeat.st
    if (defeated_)
        return true;

    if (aii.GetStorehouses().empty()) {
        aii.Surrender();
        Chat(_("I surrender"));
        defeated_ = true;
        return true;
    }

    return false;
}

void Beowulf::Chat(const std::string& message) const
{
    GAMECLIENT.GetMainPlayer().sendMsgAsync(
                new GameMessage_Chat(playerId, CD_ALL, message));
}

} // namespace beowulf
