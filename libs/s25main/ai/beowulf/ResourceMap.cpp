// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/ResourceMap.h"
#include "ai/beowulf/ProductionConsts.h"
#include "ai/AIInterface.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noTree.h"
#include "nodeObjs/noAnimal.h"
#include "gameData/GameConsts.h"
#include "EventManager.h"

#include <boost/foreach.hpp>
#include <boost/lambda/bind.hpp>

namespace beowulf {

ResourceMapNode::ResourceMapNode()
{
    memset(resources, 0, sizeof(resources));
    for (unsigned i = 0; i < BResourceCount; ++i)
        lastUpdate[i] = std::numeric_limits<unsigned>::max();
}

ResourceMap::ResourceMap(AIInterface& aii, unsigned maxAge)
    : aii_(aii), maxAge_(maxAge)
{
    memset(resources_, 0, sizeof(resources_));
    Resize(aii_.gwb.GetSize());
}

unsigned ResourceMap::Get(const MapPoint &pt, BResourceType type)
{
    const ResourceMapNode& node = (*this)[pt];
    unsigned age = node.lastUpdate[type];
    unsigned gf = aii_.gwb.GetEvMgr().GetCurrentGF();

    if (age == std::numeric_limits<unsigned>::max() || age + maxAge_ < gf)
        Update(pt, type);

    return node.resources[type];
}

void ResourceMap::Refresh()
{
    const GameWorldBase& world = aii_.gwb;

    memset(resources_, 0, sizeof(resources_));

    RTTR_FOREACH_PT(MapPoint, GetSize()) {
        if (!aii_.IsOwnTerritory(pt))
            continue;

        const Resource& res = world.GetNode(pt).resources;
        switch (res.getType()) {
        case Resource::Iron:
            resources_[BResourceIron] += res.getAmount();
            break;
        case Resource::Gold:
            resources_[BResourceGold] += res.getAmount();
            break;
        case Resource::Coal:
            resources_[BResourceCoal] += res.getAmount();
            break;
        case Resource::Granite:
            resources_[BResourceGranite] += res.getAmount();
            break;
        case Resource::Water:
            resources_[BResourceWater] += res.getAmount();
            break;
        case Resource::Fish:
            resources_[BResourceFish] += res.getAmount();
            break;
        default: break;
        }

        DescIdx<TerrainDesc> t1 = world.GetNode(pt).t1;
        if (world.GetDescription().get(t1).Is(ETerrain::Walkable)) {
            NodalObjectType no = world.GetNO(pt)->GetType();

            if (no == NOP_NOTHING || no == NOP_ENVIRONMENT) {
                if (!world.IsOnRoad(pt) && world.IsOfTerrain(pt, boost::lambda::bind(&TerrainDesc::IsVital, boost::lambda::_1))) {
                    resources_[BResourcePlantSpace_2]++;
                    resources_[BResourcePlantSpace_6]++;
                }
            } else if (no == NOP_TREE) {
                if (world.GetSpecObj<noTree>(pt)->ProducesWood())
                    resources_[BResourceWood]++;
            } else if (no == NOP_GRANITE) {
                resources_[BResourceStone]++;
            }
        }

        BOOST_FOREACH(const noBase* fig, world.GetFigures(pt)) {
            if (fig->GetType() == NOP_ANIMAL) {
                if (static_cast<const noAnimal*>(fig)->CanHunted())
                    resources_[BResourceHuntableAnimals]++;
            }
        }
    }
}

unsigned ResourceMap::Get(BResourceType type) const
{
    return resources_[type];
}

void ResourceMap::Update(const MapPoint &pt, BResourceType type)
{
    RTTR_Assert(aii_.IsOwnTerritory(pt));

    ResourceMapNode& node = (*this)[pt];
    const GameWorldBase& world = aii_.gwb;
    unsigned gf = aii_.gwb.GetEvMgr().GetCurrentGF();

    // We gather resource information for types with the same radius together.
    switch (type) {
    // Radius 1
    case BResourceWater:
    {
        node.resources[BResourceWater] = 0;

        const Resource& res = world.GetNode(pt).resources;
        if (res.getType() == Resource::Water)
            node.resources[BResourceWater] = res.getAmount();

        node.lastUpdate[BResourceWater] = gf;
    } break;

    // Radius 2
    case BResourceIron:
    case BResourceGold:
    case BResourceCoal:
    case BResourceGranite:
    case BResourcePlantSpace_2:
    {
        node.resources[BResourceIron] = 0;
        node.resources[BResourceGold] = 0;
        node.resources[BResourceCoal] = 0;
        node.resources[BResourceGranite] = 0;
        node.resources[BResourcePlantSpace_2] = 0;

        BOOST_FOREACH(const MapPoint& p, world.GetPointsInRadiusWithCenter(pt, MINER_RADIUS))
        {
            const Resource& res = world.GetNode(pt).resources;
            switch (res.getType()) {
            case Resource::Iron:
                node.resources[BResourceIron] += res.getAmount();
                break;
            case Resource::Gold:
                node.resources[BResourceGold] += res.getAmount();
                break;
            case Resource::Coal:
                node.resources[BResourceCoal] += res.getAmount();
                break;
            case Resource::Granite:
                node.resources[BResourceGranite] += res.getAmount();
                break;
            default: break;
            }

            NodalObjectType no = world.GetNO(p)->GetType();
            DescIdx<TerrainDesc> t1 = world.GetNode(p).t1;
            if (world.GetDescription().get(t1).Is(ETerrain::Walkable) &&
                    (no == NOP_NOTHING || no == NOP_ENVIRONMENT) &&
                    !world.IsOnRoad(p) &&
                    world.IsOfTerrain(p, boost::lambda::bind(&TerrainDesc::IsVital, boost::lambda::_1)))
            {
                node.resources[BResourcePlantSpace_2]++;
            }
        }

        node.lastUpdate[BResourceIron] = gf;
        node.lastUpdate[BResourceGold] = gf;
        node.lastUpdate[BResourceCoal] = gf;
        node.lastUpdate[BResourceGranite] = gf;
        node.lastUpdate[BResourcePlantSpace_2] = gf;
    } break;

    // Radius 6
    case BResourceWood:
    case BResourcePlantSpace_6:
    {
        node.resources[BResourceWood] = 0;
        node.resources[BResourcePlantSpace_6] = 0;

        BOOST_FOREACH(const MapPoint& p, GetPointsInRadiusWithCenter(pt, 6))
        {
            DescIdx<TerrainDesc> t1 = world.GetNode(p).t1;
            if (world.GetDescription().get(t1).Is(ETerrain::Walkable)) {
                NodalObjectType no = world.GetNO(p)->GetType();

                if (no == NOP_NOTHING || no == NOP_ENVIRONMENT) {
                    if(!world.IsOnRoad(p) && world.IsOfTerrain(p, boost::lambda::bind(&TerrainDesc::IsVital, boost::lambda::_1)))
                        node.resources[BResourcePlantSpace_6]++;
                } else if (no == NOP_TREE) {
                    if (world.GetSpecObj<noTree>(p)->ProducesWood())
                        node.resources[BResourceWood]++;
                }
            }
        }

        node.lastUpdate[BResourceWood] = gf;
        node.lastUpdate[BResourcePlantSpace_6] = gf;
    } break;

    // Radius 7
    case BResourceFish:
    {
        node.resources[BResourceFish] = 0;

        BOOST_FOREACH(const MapPoint& p, GetPointsInRadiusWithCenter(pt, FISHER_RADIUS))
        {
            const Resource& res = world.GetNode(p).resources;
            if (res.getType() == Resource::Fish)
                node.resources[BResourceFish] += res.getAmount();
        }

        node.lastUpdate[BResourceFish] = gf;
    } break;

    // Radius 8
    case BResourceStone:
    {
        node.resources[BResourceStone] = 0;

        BOOST_FOREACH(const MapPoint& p, GetPointsInRadiusWithCenter(pt, STONEMASON_RADIUS))
        {
            NodalObjectType no = world.GetNO(p)->GetType();
            DescIdx<TerrainDesc> t1 = world.GetNode(p).t1;
            if (world.GetDescription().get(t1).Is(ETerrain::Walkable)) {
                if (no == NOP_GRANITE) {
                    node.resources[BResourceStone]++;
                }
            }
        }

        node.lastUpdate[BResourceStone] = gf;
    } break;

    // Radius 20
    case BResourceHuntableAnimals:
    {
        node.resources[BResourceHuntableAnimals] = 0;

        BOOST_FOREACH(const MapPoint& p, GetPointsInRadiusWithCenter(pt, 20))
        {
            BOOST_FOREACH(const noBase* fig, world.GetFigures(p)) {
                if(fig->GetType() == NOP_ANIMAL) {
                    if(!static_cast<const noAnimal*>(fig)->CanHunted())
                        continue;
                    if(world.FindHumanPath(pt, static_cast<const noAnimal*>(fig)->GetPos(), 25) != 0xFF)
                         node.resources[BResourceHuntableAnimals]++;
                }
            }
        }

        node.lastUpdate[BResourceHuntableAnimals] = gf;
    } break;
    default: RTTR_Assert(false);
    }
}

} // namespace beowulf
