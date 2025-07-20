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

HeadlessGame::HeadlessGame(const Settings& settings)
: game(settings.ggs, std::make_unique<EventManager>(0), GeneratePlayerInfo(settings.ais))
, world(game.world_)
, em(*static_cast<EventManager*>(game.em_.get()))
, settings_(settings)
{
}

void HeadlessGame::Start()
{
    MapLoader loader(world);
    if(!loader.Load(settings_.map))
        throw std::runtime_error("Could not load " + settings_.map.string());

    players_.clear();
    for(unsigned playerId = 0; playerId < world.GetNumPlayers(); ++playerId)
        players_.push_back(AIFactory::Create(world.GetPlayer(playerId).aiInfo, playerId, world));

    world.InitAfterLoad();
    
    game.Start(false);
}

bool HeadlessGame::IsFinished() const
{
    if (em.GetCurrentGF() > settings_.maxGf)
        return true;

    if (AgentPlayer().IsDefeated())
        return true;

    return game.IsGameFinished();
}

void HeadlessGame::RunNextNWGF()
{
    static constexpr int NetworkFrameInterval = 20;

    for (int i = 0; i < (NetworkFrameInterval - 1); ++i) {
        for(auto& player : players_)
            player->RunGF(em.GetCurrentGF(), false);

        game.RunGF();
    }

    for (auto& player : players_) {
        PlayerGameCommands cmds;
        cmds.gcs = player->FetchGameCommands();
        for(const gc::GameCommandPtr& gc : cmds.gcs)
            gc->Execute(world, player->GetPlayerId());
    }

    for(auto& player : players_)
        player->RunGF(em.GetCurrentGF(), true);

    game.RunGF();
}

GamePlayer& HeadlessGame::AgentPlayer()
{
    return world.GetPlayer(settings_.agentIdx);
}

const GamePlayer& HeadlessGame::AgentPlayer() const
{
    return world.GetPlayer(settings_.agentIdx);
}

AIInterface& HeadlessGame::AII()
{
    return players_[settings_.agentIdx]->getAIInterface();
}

const AIInterface& HeadlessGame::AII() const
{
    return players_[settings_.agentIdx]->getAIInterface();
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
            case AI::Type::Beowulf: pi.name = "Beowulf " + std::to_string(ret.size()); break;
            case AI::Type::Dummy:
            default: pi.name = "Dummy " + std::to_string(ret.size()); break;
        }
        pi.nation = Nation::Romans;
        pi.team = Team::None;
        ret.push_back(pi);
    }
    return ret;
}

} // namespace beowulf