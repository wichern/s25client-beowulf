// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIPlayer.h"
#include "notifications/Subscription.h"

#include <vector>

struct BuildingNote;
struct ExpeditionNote;
struct NodeNote;
struct PlayerNodeNote;
struct ResourceNote;
struct RoadNote;
struct ShipNote;
struct ToolNote;

namespace beowulf {

class Beowulf : public AIPlayer
{
public:
    Beowulf(const unsigned char playerId,
            const GameWorldBase& gwb,
            const AI::Level level);
    ~Beowulf() override;

    void RunGF(const unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned /*sendPlayerId*/, ChatDestination, const std::string& /*msg*/) override {}

private:
    void OnNotification(const BuildingNote& note);
    void OnNotification(const ExpeditionNote& note);
    void OnNotification(const NodeNote& note);
    void OnNotification(const PlayerNodeNote& note);
    void OnNotification(const RoadNote& note);
    void OnNotification(const ShipNote& note);
    void OnNotification(const ResourceNote& note);
    void OnNotification(const ToolNote& note);
    std::vector<Subscription> notificationSubscriptions_;
};

} // namespace beowulf