// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TrainingGame.h"
#include "GlobalGameSettings.h"
#include "world/MapLoader.h"
#include "factories/AIFactory.h"
#include "Savegame.h"
#include "network/PlayerGameCommands.h"
#include "world/GameWorld.h"
#include "PlayerInfo.h"
#include "gameTypes/MapInfo.h"

std::vector<PlayerInfo> GeneratePlayerInfo(const std::vector<AI::Info>& ais);
std::vector<PlayerInfo> GeneratePlayerInfo(const std::vector<AI::Info>& ais)
{
    std::vector<PlayerInfo> ret;
    for (const AI::Info& ai : ais)
    {
        PlayerInfo pi;
        pi.ps = PlayerState::Occupied;
        pi.aiInfo = ai;
        pi.name = "AIJH " + std::to_string(ret.size());
        pi.nation = Nation::Romans;
        pi.team = Team::None;
        ret.push_back(pi);
    }
    return ret;
}

TrainingGame::TrainingGame(const boost::filesystem::path& map, const std::vector<AI::Info>& ais)
 : map_(map)
 , game_(GlobalGameSettings(), std::make_unique<TrainingEventManager>(), GeneratePlayerInfo(ais))
 , world_(game_.world_)
 , em_(*static_cast<TrainingEventManager*>(game_.em_.get()))
{
    MapLoader loader(world_);
    if(!loader.Load(map))
        throw std::runtime_error("Could not load " + map.string());

    players_.clear();
    for(unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId)
        players_.push_back(AIFactory::Create(world_.GetPlayer(playerId).aiInfo, playerId, world_));

    world_.InitAfterLoad();
}

TrainingGame::~TrainingGame()
{
    Close();
}

void TrainingGame::Run(unsigned maxGF)
{
    while(em_.GetCurrentGF() < maxGF)
    {
        bool isnfw = em_.GetCurrentGF() % 10 == 0;

        if(isnfw) {        
            AsyncChecksum checksum;
            if (replay_.IsRecording())
                checksum = AsyncChecksum::create(game_);
            for (unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId) {
                world_.GetPlayer(playerId);
                AIPlayer* player = players_[playerId].get();
                PlayerGameCommands cmds;
                cmds.gcs = player->FetchGameCommands();

                if(replay_.IsRecording() && !cmds.gcs.empty()) {
                    cmds.checksum = checksum;
                    replay_.AddGameCommand(em_.GetCurrentGF(), playerId, cmds);
                }

                for (gc::GameCommandPtr gc : cmds.gcs)
                    gc->Execute(world_, player->GetPlayerId());
            }
        }

        for (auto& player : players_)
            player->RunGF(em_.GetCurrentGF(), isnfw);

        em_.ExecuteNextEvent(em_.GetCurrentGF() + 1);
        // @todo: Recalc stats
        
        if(replay_.IsRecording())
            replay_.UpdateLastGF(em_.GetCurrentGF());
    }
}

void TrainingGame::Close()
{
    if(replay_.IsRecording())
        replay_.StopRecording();
    replay_.Close();
}

void TrainingGame::StartReplay(const boost::filesystem::path& path, unsigned random_init)
{
    MapInfo mapInfo;
    mapInfo.filepath = map_;
    mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum);
    mapInfo.type = MapType::OldMap;

    replay_.random_init = random_init;
    for(unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId)
        replay_.AddPlayer(world_.GetPlayer(playerId));
    replay_.ggs = game_.ggs_;
    if(!replay_.StartRecording(path, mapInfo))
        throw std::runtime_error("Replayfile could not be opened!");
}

void TrainingGame::SaveGame(const boost::filesystem::path& savegame) const
{
    Savegame save;
    for(unsigned playerId = 0; playerId < world_.GetNumPlayers(); ++playerId)
        save.AddPlayer(world_.GetPlayer(playerId));
    save.ggs = game_.ggs_;
    save.ggs.exploration = Exploration::Disabled;  // no FOW
    save.start_gf = em_.GetCurrentGF();
    save.sgd.MakeSnapshot(game_);
    save.Save(savegame, "AI Training");
}
