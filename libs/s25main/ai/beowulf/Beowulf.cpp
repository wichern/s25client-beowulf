// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/Beowulf.h"

#include "notifications/BuildingNote.h"
#include "notifications/ExpeditionNote.h"
#include "notifications/NodeNote.h"
#include "notifications/PlayerNodeNote.h"
#include "notifications/ResourceNote.h"
#include "notifications/RoadNote.h"
#include "notifications/ShipNote.h"
#include "notifications/ToolNote.h"

#include "AsciiMap.h"

namespace beowulf {

Beowulf::Beowulf(const unsigned char playerId,
                 const GameWorldBase& gwb,
                 const AI::Level level)
    : AIPlayer(playerId, gwb, level)
{
    NotificationManager& notifications = gwb.GetNotifications();
    notificationSubscriptions_.push_back(notifications.subscribe<BuildingNote>(
        [this, playerId](const BuildingNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<ExpeditionNote>(
        [this, playerId](const ExpeditionNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<NodeNote>(
        [this](const NodeNote& note) { OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<PlayerNodeNote>(
        [this, playerId](const PlayerNodeNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<ResourceNote>(
        [this, playerId](const ResourceNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<RoadNote>(
        [this, playerId](const RoadNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<ShipNote>(
        [this, playerId](const ShipNote& note) { if(note.player == playerId) OnNotification(note); }));
    notificationSubscriptions_.push_back(notifications.subscribe<ToolNote>(
        [this, playerId](const ToolNote& note) { if(note.player == playerId) OnNotification(note); }));
}

Beowulf::~Beowulf()
{
}

void Beowulf::RunGF(const unsigned gf, bool gfisnwf)
{
    (void)gf;
    (void)gfisnwf;
}

void Beowulf::OnNotification(const BuildingNote& note)
{
    switch (note.type)
    {
    case BuildingNote::Constructed:
    {

    } break;
    case BuildingNote::Destroyed:
    {

    } break;
    case BuildingNote::Captured:
    {

    } break;
    case BuildingNote::Lost:
    {

    } break;
    case BuildingNote::NoRessources:
    {

    } break;
    case BuildingNote::LuaOrder:
    {

    } break;
    default:
    case BuildingNote::LostLand:
    {

    } break;
    }
}

void Beowulf::OnNotification(const ExpeditionNote& note)
{
    (void)note;
}

void Beowulf::OnNotification(const NodeNote& note)
{
    (void)note;
}

void Beowulf::OnNotification(const PlayerNodeNote& note)
{
    (void)note;
}

void Beowulf::OnNotification(const RoadNote& note)
{
    switch (note.type)
    {
    case RoadNote::Constructed:
    {

    } break;
    case RoadNote::ConstructionFailed:
    {
        AsciiMap dbgMap(gwb, note.pos, 10);
        dbgMap.drawPlayer(GetPlayerId());
        dbgMap.write();
        RTTR_Assert(false);
    } break;
    }
}

void Beowulf::OnNotification(const ShipNote& note)
{
    (void)note;
}

void Beowulf::OnNotification(const ResourceNote& note)
{
    (void)note;
}

void Beowulf::OnNotification(const ToolNote& note)
{
    (void)note;
}

} // namespace beowulf