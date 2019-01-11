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

#include "ai/beowulf/BuildingsBase.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"

#include "pathfinding/PathConditionRoad.h"

namespace beowulf {

bool BuildingsBase::IsRoadPossible(
        const GameWorldBase& gwb,
        const MapPoint& pos,
        Direction dir) const
{
    MapPoint dest = gwb.GetNeighbour(pos, dir);

    if (HasFlag(dest))
        return true;

    for (unsigned d = 0; d < Direction::COUNT; ++d) {
        if (HasRoad(dest, Direction(d))) {
            return false;
        }
    }

    return makePathConditionRoad(gwb, false).IsNodeOk(dest);
}

std::pair<MapPoint, unsigned> BuildingsBase::GetNearestBuilding(
        const MapBase& world,
        const MapPoint& pos,
        const std::vector<BuildingType>& types,
        Island island) const
{
    // shortest euclidean distance.
    unsigned dist = std::numeric_limits<unsigned>::max();
    MapPoint closest = MapPoint::Invalid();

    uint64_t query = 0;
    for (BuildingType type : types)
        query |= 1ul << static_cast<uint64_t>(type);

    for (const Building* building : Get()) {
        // skip buildings of different type
        if (0 == (query & (1ul << static_cast<uint64_t>(building->GetType()))))
            continue;
        // skip buildings that have no position yet.
        if (!building->GetPos().isValid())
            continue;
        // skip buildings on different islands.
        if (island != InvalidIsland && GetIsland(building->GetFlag()) != island)
            continue;

        unsigned d = world.CalcDistance(pos, building->GetPos());
        if (d < dist) {
            closest = building->GetPos();
            dist = d;
        }
    }

    return { closest, dist };
}

Building* BuildingsBase::GetGoodsDest(
        const MapBase& world,
        const Building* building,
        Island island,
        const MapPoint& pos) const
{
    struct ProductionDest {
        bool checkGroup;
        std::vector<BuildingType> types;
    };

    static const ProductionDest SUPPRESS_UNUSED GOODS_DESTINATIONS[NUM_BUILDING_TYPES] =
    {
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_HEADQUARTERS
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_BARRACKS
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_GUARDHOUSE
        { false, {  } },  // BLD_NOTHING2
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_WATCHTOWER
        { false, {  } },  // BLD_NOTHING3
        { false, {  } },  // BLD_NOTHING4
        { false, {  } },  // BLD_NOTHING5
        { false, {  } },  // BLD_NOTHING6
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_FORTRESS
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_GRANITEMINE
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_COALMINE
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_IRONMINE
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_GOLDMINE
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_LOOKOUTTOWER
        { false, {  } },  // BLD_NOTHING7
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_CATAPULT
        { true,  { BLD_SAWMILL } },       // BLD_WOODCUTTER
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_FISHERY
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_QUARRY
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_FORESTER
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_SLAUGHTERHOUSE
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_HUNTER   // the hunter produces very little food
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_BREWERY
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_ARMORY
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_METALWORKS
        { true,  { BLD_ARMORY, BLD_METALWORKS } }, // BLD_IRONSMELTER
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_CHARBURNER
        { true,  { BLD_SLAUGHTERHOUSE } },// BLD_PIGFARM
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_STOREHOUSE
        { false, {  } },  // BLD_NOTHING9
        { true,  { BLD_BAKERY } },        // BLD_MILL
        { true,  { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_BAKERY
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_SAWMILL
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_MINT
        { true,  { BLD_BAKERY, BLD_BREWERY, BLD_DONKEYBREEDER, BLD_SLAUGHTERHOUSE } }, // BLD_WELL
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_SHIPYARD
        { true,  { BLD_BAKERY, BLD_BREWERY, BLD_DONKEYBREEDER, BLD_SLAUGHTERHOUSE } }, // BLD_FARM
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_DONKEYBREEDER
        { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_HARBORBUILDING
    };

    // Get destination infos for this type of building.
    const ProductionDest& dest = GOODS_DESTINATIONS[building->GetType()];

    if (dest.checkGroup) {
        for (Building* bld : Get()) {
            if (bld->GetGroup() != building->GetGroup())
                continue;

            if (std::find(dest.types.begin(), dest.types.end(), bld->GetType()) != dest.types.end()) {
                return bld;
            }
        }
    }

    auto nearest = GetNearestBuilding(world, pos, dest.types, island);

    return nearest.first.isValid() ? Get(nearest.first) : nullptr;
}

} // namespace beowulf
