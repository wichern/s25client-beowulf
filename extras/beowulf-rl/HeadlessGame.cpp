// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "HeadlessGame.h"
#include "Settings.h"

#include "EventManager.h"
#include "GlobalGameSettings.h"
#include "PlayerInfo.h"
#include "Savegame.h"
#include "factories/AIFactory.h"
#include "network/PlayerGameCommands.h"
#include "world/GameWorld.h"
#include "world/MapLoader.h"
#include "gameTypes/MapInfo.h"
#include "gameData/GameConsts.h"

namespace beowulf {

std::vector<PlayerInfo> GeneratePlayerInfo(const std::vector<AI::Info>& ais);

HeadlessGame::HeadlessGame(Settings* settings)
: settings_(settings)
, game_(settings->ggs, std::make_unique<EventManager>(0), GeneratePlayerInfo(settings->ais))
, world_(game_.world_)
, em_(*static_cast<EventManager*>(game_.em_.get()))
{
    MapLoader loader(world_);
    if(!loader.Load(settings->map))
        throw std::runtime_error("Could not load " + settings->map.string());

    players_.clear();
    for(unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId)
        players_.push_back(AIFactory::Create(world_.GetPlayer(playerId).aiInfo, playerId, world_));

    world_.InitAfterLoad();
    
    game_.Start(false);
}

bool HeadlessGame::IsFinished(unsigned maxGF) const
{
    return em_.GetCurrentGF() < maxGF && !game_.IsGameFinished();
}

std::vector<PlayerInfo> GeneratePlayerInfo(const std::vector<AI::Info>& ais)
{
    std::vector<PlayerInfo> ret;
    for(const AI::Info& ai : ais)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.aiInfo = ai;
        switch(ai.type)
        {
            case AI::Type::Default: pi.name = "AIJH " + std::to_string(ret.size()); break;
            case AI::Type::Dummy:
            default: pi.name = "Dummy " + std::to_string(ret.size()); break;
        }
        pi.nation = Nation::Romans;
        pi.team = Team::None;
        ret.push_back(pi);
    }
    return ret;
}

void HeadlessGame::RunNextNWGF()
{
    for (int i = 0; i < 19; ++i) {

        for(auto& player : players_)
            player->RunGF(em_.GetCurrentGF(), false);

        game_.RunGF();
    }

    for(unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId)
    {
        world_.GetPlayer(playerId);
        AIPlayer* player = players_[playerId].get();
        PlayerGameCommands cmds;
        cmds.gcs = player->FetchGameCommands();

        for(const gc::GameCommandPtr& gc : cmds.gcs)
            gc->Execute(world_, player->GetPlayerId());
    }

    for(auto& player : players_)
        player->RunGF(em_.GetCurrentGF(), true);

    game_.RunGF();
}

} // namespace beowulf