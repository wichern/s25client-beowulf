// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AIGameManager.h"

#include "ai/AIPlayer.h"
#include "EventManager.h"
#include "RttrConfig.h"
#include "factories/AIFactory.h"
#include "files.h"
#include "random/Random.h"
#include "world/MapLoader.h"
#include "gameTypes/MapInfo.h"
#include "network/PlayerGameCommands.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>

#include <chrono>
#include <iomanip>

namespace bfs = boost::filesystem;
namespace bnw = boost::nowide;

AIGameManager::AIGameManager(bool createReplay, const std::vector<PlayerInfo>& playerInfos)
    : playerInfos_(playerInfos), game_(GlobalGameSettings(), 0 /* start-frame */, playerInfos_)
{
    if(createReplay)
    {
        replayInfo_ = std::make_unique<ReplayInfo>();
    }
}

bool AIGameManager::Start(std::string& mapPath)
{
    // Initialize random generator
    uint64_t random_init = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    RANDOM.Init(random_init);

    if(!RTTRCONFIG.Init())
    {
        bnw::cerr << "RTTRCONFIG.Init() failed\n";
        return EXIT_FAILURE;
    }

    MapLoader mapLoader(game_.world_);
    if(!mapLoader.Load(mapPath))
    {
        bnw::cerr << "Could not load map: " << mapPath << "\n";
        return false;
    }

    for(unsigned i = 0; i < playerInfos_.size(); ++i)
        game_.AddAIPlayer(AIFactory::Create(playerInfos_[i].aiInfo, i, game_.world_));

    game_.Start(false /* startFromSave */);

    return InitReplay(mapPath, random_init);
}

bool AIGameManager::InitReplay(std::string& mapPath, uint64_t random_init)
{
    if(replayInfo_)
    {
        replayInfo_->filename = "ai-battle " + s25util::Time::FormatTime("%Y-%m-%d_%H-%i-%s") + ".rpl";
        replayInfo_->replay.random_init = random_init;
        replayInfo_->replay.ggs = game_.ggs_;
        for(const auto& pi : playerInfos_)
            replayInfo_->replay.AddPlayer(pi);

        MapInfo mapInfo;
        mapInfo.type = MapType::OldMap;
        mapInfo.title = "AI Battle"; // @todo: check GameServer.cpp:148 on how to set title
        mapInfo.filepath = mapPath;

        if(!mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum))
        {
            bnw::cerr << "Could not load map data from " << mapPath << "\n";
            return false;
        }
        bfs::path luaFilePath = bfs::path(mapInfo.filepath).replace_extension("lua");
        if(bfs::is_regular_file(luaFilePath))
        {
            if(!mapInfo.luaData.CompressFromFile(luaFilePath, &mapInfo.luaChecksum))
            {
                bnw::cerr << "Could not load lua data from " << mapPath << "\n";
                return false;
            }
            mapInfo.luaFilepath = luaFilePath;
        }

        if(!mapInfo.verifySize())
        {
            bnw::cerr << "Map is too large: " << mapPath << "\n";
            return false;
        }

        if(!replayInfo_->replay.StartRecording(RTTRCONFIG.ExpandPath(s25::folders::replays) / replayInfo_->filename,
                                               mapInfo))
        {
            bnw::cerr << "Could not start recording replay\n";
            return false;
        }
    }

    return true;
}

bool AIGameManager::Run()
{
    if(game_.IsGameFinished())
        return false;

    bool isnwf = (game_.em_->GetCurrentGF() % 5 == 0);
    for(unsigned player = 0; player < playerInfos_.size(); ++player)
    {
        game_.GetAIPlayer(player)->RunGF(game_.em_->GetCurrentGF(), isnwf);

        auto gcs = game_.GetAIPlayer(player)->FetchGameCommands();
        for(gc::GameCommandPtr& gc : gcs)
            gc->Execute(game_.world_, player);
        if(replayInfo_ && !gcs.empty() && replayInfo_->replay.IsRecording())
        {
            PlayerGameCommands playergcs;
            playergcs.gcs = gcs;
            replayInfo_->replay.AddGameCommand(game_.em_->GetCurrentGF(), player, playergcs);
        }
    }

    game_.RunGF();

    if(game_.em_->GetCurrentGF() % 500 == 0)
    {
        bnw::cout << "GF " << game_.em_->GetCurrentGF() << "\n";
        for(unsigned i = 0; i < playerInfos_.size(); ++i)
            bnw::cout << playerInfos_[i].name << ": Country: " << std::setw(5)
                      << game_.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Country)
                      << ", Buildings: " << std::setw(3)
                      << game_.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Buildings)
                      << ", Military: " << std::setw(3)
                      << game_.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Military)
                      << ", Gold: " << std::setw(3)
                      << game_.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Gold)
                      << ", Productivity: " << std::setw(2)
                      << game_.GetAIPlayer(i)->player.GetStatisticCurrentValue(StatisticType::Productivity) << "\n";
    }

    return true;
}

void AIGameManager::Stop()
{
    if(replayInfo_)
    {
        if(replayInfo_->replay.IsRecording())
        {
            replayInfo_->replay.UpdateLastGF(game_.em_->GetCurrentGF());
            replayInfo_->replay.StopRecording();
        }
        replayInfo_->replay.Close();
        bnw::cout << "Replay written to " << replayInfo_->filename << "\n";
        replayInfo_.reset();
    }
}
