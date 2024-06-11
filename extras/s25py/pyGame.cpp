// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyGame.h"
#include "pyPlayer.h"

#include "EventManager.h"
#include "files.h"
#include "Game.h"
#include "PlayerInfo.h"
#include "random/Random.h"
#include "RttrConfig.h"
#include "world/GameWorld.h"
#include "world/MapLoader.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;

namespace s25py
{

PyGame::PyGame(const std::string& map)
: map_(map)
{
    RTTRCONFIG.Init();
}

PyGame::~PyGame()
{
    bnw::cout << "~PyGame()" << std::endl;
}

void PyGame::AddPlayer(PyPlayer* player)
{
    if (!player)
        return;
    player->id_ = players_.size();
    player->game_ = this;
    players_.push_back(player);
}

void PyGame::ActivateReplay(bool activate)
{
    saveReplay_ = activate;
}

void PyGame::Start()
{
    if (game_)
        throw std::runtime_error("Game already started");

    // @todo: allow setting seed
    RANDOM.Init(0);

    std::vector<PlayerInfo> playerInfos;
    for (auto* player : players_)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.aiInfo = {AI::Type::Default, AI::Level::Hard};
        pi.name = player->name_;
        pi.nation = Nation::Romans;
        pi.team = Team::None;
        playerInfos.push_back(pi);
    }

    game_ = new Game(ggs_, std::make_unique<EventManager>(0), playerInfos);

    MapLoader loader(game_->world_);
    auto map_path = RTTRCONFIG.ExpandPath(map_);
    if (!loader.Load(map_path))
        throw std::runtime_error("Could not load " + map_path.string());

    game_->world_.InitAfterLoad();
}

void PyGame::Step()
{
    for (auto* player : players_)
        player->on_gameframe(false);
}

void PyGame::Stop()
{
    if (!game_)
        throw std::runtime_error("Game not started");

    delete game_;
    game_ = nullptr;
}

}  // namespace s25py