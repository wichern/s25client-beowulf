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
#include <boost/nowide/fstream.hpp>

#include <memory> /* std::unique_ptr */

static const MapPoint BiggerWorld_HQPoint(12, 11);
static const MapPoint BiggerWorld_HQFlag(13, 12);

#define BEOWULF_ENABLE_ALL

void Proceed(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world,
        unsigned& player,
        TestEventManager& em);

bool ConstructBuilding(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world,
        unsigned& player,
        TestEventManager& em,
        BuildingType type,
        const MapPoint& pos,
        bool wait_for_site);

bool CompareBuildingsWithWorld(
        std::unique_ptr<AIPlayer>& ai,
        GameWorldGame& world);

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
