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

#include "ai/beowulf/Building.h"
#include "ai/beowulf/Buildings.h"

#include "gameData/BuildingConsts.h"

#include <limits>

namespace beowulf {

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
    { true,  { BLD_MILL, BLD_BREWERY, BLD_DONKEYBREEDER, BLD_SLAUGHTERHOUSE } }, // BLD_FARM
    { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },    // BLD_DONKEYBREEDER
    { false, { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING } },  // BLD_HARBORBUILDING
};

Building::Building(
        Buildings& buildings,
        BuildingType type,
        State state)
    : buildings_(buildings),
      pt_(MapPoint::Invalid()),
      type_(type),
      state_(state),
      group_(InvalidProductionGroup)
{

}

const MapPoint& Building::GetPt() const
{
    return pt_;
}

MapPoint Building::GetFlag() const
{
    if (pt_.isValid())
        return buildings_.GetWorld().GetNeighbour(pt_, Direction::SOUTHEAST);
    return MapPoint::Invalid();
}

BuildingType Building::GetType() const
{
    return type_;
}

Building::State Building::GetState() const
{
    return state_;
}

pgroup_id_t Building::GetGroup() const
{
    return group_;
}

BuildingQuality Building::GetQuality() const
{
    return BUILDING_SIZE[type_];
}

unsigned Building::GetDistance(const MapPoint& pt) const
{
    return buildings_.GetWorld().CalcDistance(GetPt(), pt);
}

const std::vector<BuildingType>& Building::GetDestTypes(bool& checkGroup) const
{
    const ProductionDest& dest = GOODS_DESTINATIONS[type_];
    checkGroup = dest.checkGroup;
    return dest.types;
}

bool Building::IsProduction() const
{
    return type_ == BLD_WOODCUTTER ||
           type_ == BLD_FORESTER ||
           type_ == BLD_SLAUGHTERHOUSE ||
           type_ == BLD_BREWERY ||
           type_ == BLD_ARMORY ||
           type_ == BLD_IRONSMELTER ||
           type_ == BLD_PIGFARM ||
           type_ == BLD_MILL ||
           type_ == BLD_BAKERY ||
           type_ == BLD_SAWMILL ||
           type_ == BLD_MINT ||
           type_ == BLD_WELL ||
           type_ == BLD_FARM ||
           type_ == BLD_DONKEYBREEDER ||
           type_ == BLD_QUARRY;
}

bool Building::IsStorage() const
{
    return type_ == BLD_HEADQUARTERS || type_ == BLD_STOREHOUSE || type_ == BLD_HARBORBUILDING;
}

} // namespace beowulf
