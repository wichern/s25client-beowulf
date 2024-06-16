// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyGame.h"
#include "pyPlayer.h"

#include "AsyncChecksum.h"
#include "EventManager.h"
#include "Game.h"
#include "GlobalGameSettings.h"
#include "PlayerInfo.h"
#include "RttrConfig.h"
#include "ai/AIPlayer.h"
#include "files.h"
#include "network/PlayerGameCommands.h"
#include "random/Random.h"
#include "world/GameWorld.h"
#include "world/MapLoader.h"
#include "gameTypes/AIInfo.h"
#include "gameTypes/GameTypesOutput.h"
#include "gameTypes/MapInfo.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;

namespace s25py {

PyGame::PyGame(const std::string& mapPath, std::string replayPath, GameObjective objective, unsigned maxGF,
               unsigned randomSeed, unsigned nwfInterval)
    : mapPath_(mapPath), replayPath_(replayPath), objective_(objective), maxGF_(maxGF), randomSeed_(randomSeed),
      nwfInterval_(nwfInterval)
{
    RTTRCONFIG.Init();

    bnw::cout << "Game(" << mapPath_ << ", " << replayPath_ << ", " << objective_ << ", " << maxGF_ << ", "
              << randomSeed_ << ", " << nwfInterval_ << ")" << std::endl;
}

PyGame::~PyGame()
{
    bnw::cout << "~PyGame()" << std::endl;
    if(game_)
    {
        if(replay_.IsRecording())
        {
            replay_.StopRecording();
        }

        replay_.Close();

        delete game_;
        game_ = nullptr;
    }
}

void PyGame::AddPlayer(PyPlayer* player)
{
    if(!player)
        return;
    player->id_ = players_.size();
    player->game_ = this;
    players_.push_back(player);
}

void PyGame::Start()
{
    RANDOM.Init(randomSeed_);

    std::vector<PlayerInfo> playerInfos;
    for(auto* player : players_)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.aiInfo = {AI::Type::Default, AI::Level::Hard};
        pi.name = player->name_;
        pi.nation = Nation::Romans;
        pi.team = Team::None;
        playerInfos.push_back(pi);
    }

    GlobalGameSettings ggs;
    ggs.objective = objective_;

    game_ = new Game(ggs, std::make_unique<EventManager>(0), playerInfos);

    MapLoader loader(game_->world_);
    auto mapPath = RTTRCONFIG.ExpandPath(mapPath_);
    if(!loader.Load(mapPath))
        throw std::runtime_error("Could not load " + mapPath.string());

    game_->world_.InitAfterLoad();
    game_->Start(false);

    // Assign AIPlayer objects to PyPlayer objects
    for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
    {
        AIPlayer* aip = game_->GetAIPlayer(playerId);
        if(!aip)
            throw std::runtime_error("No AI player with ID " + std::to_string(playerId));
        players_[playerId]->player_ = aip;
    }

    // @todo: Subroutine: startReplay(mapPath)
    // Remove old replay
    bfs::remove(replayPath_);

    MapInfo mapInfo;
    mapInfo.filepath = mapPath;
    mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum);
    mapInfo.type = MapType::OldMap;

    replay_.random_init = randomSeed_;
    for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
        replay_.AddPlayer(game_->world_.GetPlayer(playerId));
    replay_.ggs = game_->ggs_;
    if(!replay_.StartRecording(replayPath_, mapInfo))
        throw std::runtime_error("Replayfile could not be opened!");
}

bool PyGame::Step()
{
    if(!game_)
        Start();

    if(game_->em_->GetCurrentGF() >= maxGF_)
        return false;

    if(game_->IsGameFinished())
        return false;

    bool isnfw = game_->em_->GetCurrentGF() % nwfInterval_ == 0;
    AsyncChecksum checksum;

    if(isnfw)
    {
        if(replay_.IsRecording())
            checksum = AsyncChecksum::create(*game_);

        for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
        {
            PyPlayer* player = players_[playerId];
            PlayerGameCommands cmds;
            cmds.gcs = player->player_->FetchGameCommands();

            if(replay_.IsRecording() && !cmds.gcs.empty())
            {
                cmds.checksum = checksum;
                replay_.AddGameCommand(game_->em_->GetCurrentGF(), playerId, cmds);
            }

            for(const gc::GameCommandPtr& gc : cmds.gcs)
                gc->Execute(game_->world_, player->player_->GetPlayerId());
        }
    }

    for(auto& player : players_)
        player->RunGF(game_->em_->GetCurrentGF(), isnfw);

    game_->RunGF();

    if(replay_.IsRecording())
        replay_.UpdateLastGF(game_->em_->GetCurrentGF());

    return true;
}

} // namespace s25py