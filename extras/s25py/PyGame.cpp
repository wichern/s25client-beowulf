// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PyGame.h"
#include "PyPlayer.h"

#include "ai/AIPlayer.h"
#include "ai/s25py/AIPlayerPython.h"
#include "AsyncChecksum.h"
#include "EventManager.h"
#include "factories/AIFactory.h"
#include "files.h"
#include "Game.h"
#include "gameTypes/AIInfo.h"
#include "gameTypes/GameTypesOutput.h"
#include "gameTypes/MapInfo.h"
#include "GlobalGameSettings.h"
#include "network/PlayerGameCommands.h"
#include "random/Random.h"
#include "RttrConfig.h"
#include "world/GameWorld.h"
#include "world/MapLoader.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;
namespace py = pybind11;

namespace s25py {

PyGame::PyGame(const std::string& mapPath, std::string replayPath, GameObjective objective,
               unsigned randomSeed, unsigned nwfInterval)
    : mapPath_(mapPath), replayPath_(replayPath), objective_(objective), randomSeed_(randomSeed),
      nwfInterval_(nwfInterval)
{
    RTTRCONFIG.Init();
    RANDOM.Init(randomSeed);

    bnw::cout << "Game(map: " << mapPath_ << ", replay: " << replayPath_ << ", objective: " << objective_ << ", seed: "
              << randomSeed << ", nwfInterval: " << nwfInterval_ << ")" << std::endl;
}

PyGame::~PyGame()
{
    if(game_)
    {
        if(replay_.IsRecording())
            replay_.StopRecording();

        replay_.Close();

        delete game_;
        game_ = nullptr;
    }
}

void PyGame::AddPlayer(const std::string& name, AI::Level level, std::shared_ptr<PyPlayer> player)
{
    AddPlayerInternal(name, AI::Type::Python, level, player);
}

void PyGame::AddPlayerAIJH(const std::string& name, AI::Level level)
{
    AddPlayerInternal(name, AI::Type::Default, level, nullptr);
}

void PyGame::AddPlayerDummy(const std::string& name, AI::Level level)
{
    AddPlayerInternal(name, AI::Type::Dummy, level, nullptr);
}

void PyGame::AddPlayerInternal(const std::string& name, AI::Type type, AI::Level level, std::shared_ptr<PyPlayer> player)
{
    PlayerInfo pi;
    pi.ps = PlayerState::AI;
    pi.aiInfo.type = type;
    pi.aiInfo.level = level;
    pi.name = name;
    pi.nation = Nation::Romans;
    pi.team = Team::None;
    playerInfos_.push_back({pi, player});
}

void PyGame::Run(unsigned maxGF)
{
    Start();

    while(true) {
        if (getCurrentGF() >= maxGF) {
            std::cout << "Game stopped after " << maxGF << " GFs." << std::endl;
            break;
        }

        if (game_->IsGameFinished()) {
            std::cout << "Game finished after " << getCurrentGF() << " GFs." << std::endl;
            break;
        }

        Step();
    }
}

void PyGame::Start()
{
    GlobalGameSettings ggs;
    ggs.objective = objective_;

    std::vector<PlayerInfo> pis;
    for (const auto& pi : playerInfos_)
        pis.push_back(pi.first);
    
    game_ = new Game(ggs, std::make_unique<EventManager>(0), pis);

    MapLoader loader(game_->world_);
    auto mapPath = RTTRCONFIG.ExpandPath(mapPath_);
    if(!loader.Load(mapPath))
        throw std::runtime_error("Could not load " + mapPath.string());

    for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
    {
        auto& pi = playerInfos_[playerId];
        if(pi.second)
            game_->AddAIPlayer(std::make_unique<s25py::AIPlayerPython>(playerId, game_->world_, pi.first.aiInfo.level, py::cast(pi.second)));
        else
            game_->AddAIPlayer(AIFactory::Create(pi.first.aiInfo, playerId, game_->world_));
    }

    game_->world_.InitAfterLoad();

    game_->Start(false);

    // @todo: Subroutine: startReplay(mapPath)
    if(!replayPath_.empty()) {
        // Remove old replay
        bfs::remove(replayPath_);

        MapInfo mapInfo;
        mapInfo.filepath = mapPath;
        mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum);
        mapInfo.type = MapType::OldMap;

        for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
            replay_.AddPlayer(game_->world_.GetPlayer(playerId));
        replay_.ggs = game_->ggs_;
        if(!replay_.StartRecording(replayPath_, mapInfo, randomSeed_))
            throw std::runtime_error("Replayfile (" + replayPath_ + ") could not be opened!");
    }
}

void PyGame::Step()
{
    bool isnfw = getCurrentGF() % nwfInterval_ == 0;
    AsyncChecksum checksum;

    if(isnfw)
    {
        if(replay_.IsRecording())
            checksum = AsyncChecksum::create(*game_);

        for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
        {
            auto& player = game_->aiPlayers_[playerId];
            PlayerGameCommands cmds;
            cmds.gcs = player.FetchGameCommands();

            if(replay_.IsRecording() && !cmds.gcs.empty())
            {
                cmds.checksum = checksum;
                replay_.AddGameCommand(getCurrentGF(), playerId, cmds);
            }

            for(const gc::GameCommandPtr& gc : cmds.gcs)
                gc->Execute(game_->world_, playerId);
        }
    }

    for(auto& player : game_->aiPlayers_)
        player.RunGF(getCurrentGF(), isnfw);

    game_->RunGF();

    if(replay_.IsRecording())
        replay_.UpdateLastGF(getCurrentGF());
}

unsigned PyGame::getCurrentGF() const
{
    return game_->em_->GetCurrentGF();
}

std::vector<unsigned> PyGame::getPlayerBuildings() const
{
    std::vector<unsigned> ret;
    ret.reserve(game_->world_.GetNumPlayers());

    for(unsigned playerId = 0; playerId < game_->world_.GetNumPlayers(); ++playerId)
    {
        const GamePlayer& player = game_->world_.GetPlayer(playerId);
        ret.push_back(player.GetStatisticCurrentValue(StatisticType::Buildings));
    }

    return ret;
}

} // namespace s25py