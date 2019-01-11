// Copyright (c) 2016 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#pragma once

#include "rttrDefines.h" // IWYU pragma: keep
#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Types.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noFlag.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"

#include "RttrConfig.h"
#include "files.h"
#include "ogl/glArchivItem_Map.h"
#include "world/MapLoader.h"
#include "Replay.h"
#include "gameTypes/MapInfo.h"
#include "libutil/ucString.h"
#include "network/PlayerGameCommands.h"

#include <boost/nowide/fstream.hpp>
#include <boost/filesystem.hpp>

#include <memory> /* std::unique_ptr */

#define DISABLE_ALL_BEOWULF_TESTS
#define ENABLE_BEOWULF_FIGHTS

template<class Condition>
bool Proceed(
        Condition condition,
        std::vector<AIPlayer*> player,
        TestEventManager& em,
        GameWorldGame& world,
        unsigned maxGf = 0,
        unsigned* totalGf = nullptr)
{
    unsigned startGf = em.GetCurrentGF();
    while (maxGf == 0 || em.GetCurrentGF() < (startGf + maxGf)) {
        if (condition()) {
            if (totalGf)
                *totalGf = em.GetCurrentGF() - startGf;
            return true;
        }

        bool isnfw = em.GetCurrentGF() % 10;

        if (isnfw) {
            for (AIPlayer* p : player) {
                for (gc::GameCommandPtr gc : p->FetchGameCommands())
                    gc->Execute(world, p->GetPlayerId());
            }
        }

        for (AIPlayer* p : player)
            p->RunGF(em.GetCurrentGF(), isnfw);

        em.ExecuteNextEvent(em.GetCurrentGF() + 1);
    }

    if (totalGf)
        *totalGf = em.GetCurrentGF() - startGf;
    return false;
}

void Proceed(
        std::vector<AIPlayer*> player,
        TestEventManager& em,
        GameWorldGame& world);

bool ConstructBuilding(
        AIPlayer* ai,
        GameWorldGame& world,
        TestEventManager& em,
        BuildingType type,
        const MapPoint& pos,
        bool wait_for_site);

bool CompareBuildingsWithWorld(
        AIPlayer* ai,
        GameWorldGame& world);

bool IsOutsidePlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& points);

bool IsInsidePlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& points);

bool EqualsPlayerTerritory(
        const beowulf::Beowulf* beowulf,
        const std::vector<MapPoint>& originalPoints,
        const std::vector<MapPoint>& additionalPoints);

std::vector<MapPoint> GetPlayerTerritory(
        const beowulf::Beowulf* beowulf);

bool IsConnected(
        const MapPoint& src,
        const MapPoint& dst,
        const beowulf::World& world);

bool IsConnected(
        const beowulf::Building* building,
        const beowulf::Beowulf* beowulf);

//-----------------------------------------------------------------------

struct MapTestFixture
{
    const std::string testMapPath;
    MapTestFixture() : testMapPath(RTTRCONFIG.ExpandPath(std::string(FILE_PATHS[52]) + "/Bergschlumpf.swd")) {}
};

struct LoadWorldFromFileCreator : MapTestFixture
{
    glArchivItem_Map map;
    std::vector<MapPoint> hqs;

    explicit LoadWorldFromFileCreator(const MapExtent&) {}
    bool operator()(GameWorldBase& world)
    {
        bnw::ifstream mapFile(testMapPath, std::ios::binary);
        if(map.load(mapFile, false) != 0)
            throw std::runtime_error("Could not load file " + testMapPath);
        MapLoader loader(world);
        if(!loader.Load(map, EXP_FOGOFWAR))
            throw std::runtime_error("Could not load map");
        if(!loader.PlaceHQs(world, false))
            throw std::runtime_error("Could not place HQs");
        world.InitAfterLoad();
        for(unsigned i = 0; i < world.GetNumPlayers(); i++)
            hqs.push_back(loader.GetHQPos(i));
        return true;
    }
};

struct WorldLoaded1PFixture : public WorldFixture<LoadWorldFromFileCreator, 1>
{
    using WorldFixture<LoadWorldFromFileCreator, 1>::world;
};

struct WorldLoaded2PFixture : public WorldFixture<LoadWorldFromFileCreator, 2>
{
    using WorldFixture<LoadWorldFromFileCreator, 2>::world;
};

template<const char* T_map>
struct WorldCreatorAIBattle
{
    const std::string mapPath = RTTRCONFIG.ExpandPath(std::string(FILE_PATHS[52]) + "/" + T_map);
    glArchivItem_Map map;

    explicit WorldCreatorAIBattle(const MapExtent&) {
    }

    bool operator()(GameWorldBase& world)
    {
        bnw::ifstream mapFile(mapPath, std::ios::binary);
        if (map.load(mapFile, false) != 0)
            throw std::runtime_error("Could not load file " + mapPath);

        MapLoader loader(world);
        if (!loader.Load(map, EXP_FOGOFWAR))
            throw std::runtime_error("Could not load map");

        if (!loader.PlaceHQs(world, false))
            throw std::runtime_error("Could not place HQs");

        world.InitAfterLoad();

        return true;
    }
};

template<const char* T_map, unsigned T_numPlayers, const char* T_replayPath=nullptr>
struct WorldAIBattle : public WorldFixture<WorldCreatorAIBattle<T_map>, T_numPlayers>
{
    using WorldFixture<WorldCreatorAIBattle<T_map>, T_numPlayers>::world;
    using WorldFixture<WorldCreatorAIBattle<T_map>, T_numPlayers>::em;
    using WorldFixture<WorldCreatorAIBattle<T_map>, T_numPlayers>::game;
    using WorldFixture<WorldCreatorAIBattle<T_map>, T_numPlayers>::worldCreator;
    MapInfo mapInfo;
    std::shared_ptr<Replay> replay;

    std::vector<std::shared_ptr<AIPlayer>> players;

    WorldAIBattle()
    {
        replay = std::make_shared<Replay>();

        if (T_replayPath) {
            MapInfo mapInfo;
            mapInfo.filepath = worldCreator.mapPath;
            mapInfo.mapData.CompressFromFile(mapInfo.filepath, &mapInfo.mapChecksum);
            mapInfo.type = MAPTYPE_OLDMAP;
            replay->StartRecording(T_replayPath, mapInfo);
        }
    }

    ~WorldAIBattle()
    {
        if (replay) {
            replay->StopRecording();
            replay->Close();
            std::cout << "Replay written to " << boost::filesystem::current_path().c_str() << "/" << T_replayPath << std::endl;
        }
    }

    template<class AIType>
    std::shared_ptr<AIType> CreatePlayer(AI::Type type, AI::Level level = AI::HARD)
    {
        std::shared_ptr<AIType> player(static_cast<AIType*>(AIFactory::Create(AI::Info(type, level), players.size(), world)));
        players.push_back(player);
        return player;
    }

    template<class Condition>
    bool Proceed(
            Condition condition,
            unsigned maxGf = 0,
            unsigned* totalGf = nullptr)
    {
        unsigned startGf = em.GetCurrentGF();
        while (maxGf == 0 || em.GetCurrentGF() < (startGf + maxGf)) {
            if (condition()) {
                if (totalGf)
                    *totalGf = em.GetCurrentGF() - startGf;
                return true;
            }

            bool isnfw = em.GetCurrentGF() % 10;

            if (isnfw) {
                for (unsigned i = 0; i < world.GetNumPlayers(); ++i) {
                    std::shared_ptr<AIPlayer>& player = players[i];

                    PlayerGameCommands cmds;
                    cmds.gcs = player->FetchGameCommands();
                    if (replay && replay->IsRecording() && !cmds.gcs.empty()) {
                        cmds.checksum = AsyncChecksum::create(*game);
                        replay->AddGameCommand(em.GetCurrentGF(), player->GetPlayerId(), cmds);
                    }

                    for (gc::GameCommandPtr gc : cmds.gcs) {
                        gc->Execute(world, player->GetPlayerId());
                    }
                }
            }

            for (std::shared_ptr<AIPlayer>& p : players)
                p->RunGF(em.GetCurrentGF(), isnfw);

            em.ExecuteNextEvent(em.GetCurrentGF() + 1);

            // Statistics
            for(unsigned i = 0; i < world.GetNumPlayers(); ++i)
                world.GetPlayer(i).StatisticStep();
        }

        if (totalGf)
            *totalGf = em.GetCurrentGF() - startGf;
        return false;
    }

    void DrawAsciiMap(std::ostream& out = std::cout) const
    {
        beowulf::AsciiMap map(players.front()->getAIInterface(), { 105, 26}, 20);
        map.drawResources();
        for (const std::shared_ptr<AIPlayer>& p : players) {
            map.draw(p.get());
            map.drawBorder(p->GetPlayerId());
            map.drawSoldiers(p.get());
        }
        map.write(out);
    }

    void DrawAllPlayerStatistics() const
    {
        std::vector<std::string> row = { "", "Country", "Buildings", "Inhabitants", "Merchandise", "Military", "Gold", "Productivity", "Vanquished", "Tournament" };
        beowulf::AsciiTable table(row.size());
        table.addRow(row);
        table.alignLeft(0);

        for (size_t i = 0; i < players.size(); ++i) {
            row.clear();

            const std::shared_ptr<AIPlayer>& p = players[i];
            const GamePlayer::Statistic& statistic = world.GetPlayer(p->GetPlayerId()).GetStatistic(STAT_15M);

            row.push_back("Player " + std::to_string(p->GetPlayerId()));
            for (unsigned s = 0; s < NUM_STAT_TYPES; ++s)
                row.push_back(std::to_string(statistic.data[s][statistic.currentIndex]));

            table.addRow(row);
        }

        table.write();
    }

    void DrawPlayerInventory() const
    {
        std::vector<std::string> row;
        row.push_back("Good/Job");

        for (const std::shared_ptr<AIPlayer>& p : players) {
            row.push_back("Player " + std::to_string(p->GetPlayerId()));
        }

        beowulf::AsciiTable table(row.size());
        table.addRow(row);
        table.alignLeft(0);

        for (unsigned gt = 0; gt < NUM_WARE_TYPES; ++gt) {
            if (WARE_NAMES[gt].empty())
                continue;
            row[0] = WARE_NAMES[gt];
            for (size_t i = 0; i < players.size(); ++i) {
                const std::shared_ptr<AIPlayer>& p = players[i];

                unsigned goodCount = p->getAIInterface().GetInventory()[static_cast<GoodType>(gt)];
                row[i + 1] = std::to_string(goodCount);
            }
            table.addRow(row);
        }

        table.write();
    }
};
