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

#include "ai/beowulf/Building.h"
#include "ai/beowulf/World.h"

#include "gameData/BuildingConsts.h"
#include "gameData/BuildingProperties.h"
#include "buildings/nobBaseWarehouse.h"

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

static const Building::TrafficExpected SUPPRESS_UNUSED GOODS_TRAFFIC[NUM_BUILDING_TYPES] =
{
    { 0, 0 },  // BLD_HEADQUARTERS
    { 0, 0 },  // BLD_BARRACKS
    { 0, 0 },  // BLD_GUARDHOUSE
    { 0, 0 },  // BLD_NOTHING2
    { 0, 0 },  // BLD_WATCHTOWER
    { 0, 0 },  // BLD_NOTHING3
    { 0, 0 },  // BLD_NOTHING4
    { 0, 0 },  // BLD_NOTHING5
    { 0, 0 },  // BLD_NOTHING6
    { 0, 0 },  // BLD_FORTRESS
    { 1, 1 },  // BLD_GRANITEMINE
    { 1, 1 },  // BLD_COALMINE
    { 1, 1 },  // BLD_IRONMINE
    { 1, 1 },  // BLD_GOLDMINE
    { 0, 0 },  // BLD_LOOKOUTTOWER
    { 0, 0 },  // BLD_NOTHING7
    { 0, 0 },  // BLD_CATAPULT
    { 0, 0 },  // BLD_WOODCUTTER
    { 1, 0 },  // BLD_FISHERY
    { 2, 0 },  // BLD_QUARRY
    { 0, 0 },  // BLD_FORESTER
    { 1, 1 },  // BLD_SLAUGHTERHOUSE
    { 1, 0 },  // BLD_HUNTER   // the hunter produces very little food
    { 1, 2 },  // BLD_BREWERY
    { 1, 2 },  // BLD_ARMORY
    { 1, 2 },  // BLD_METALWORKS
    { 1, 2 },  // BLD_IRONSMELTER
    { 1, 2 },  // BLD_CHARBURNER
    { 1, 2 },  // BLD_PIGFARM
    { 0, 0 },  // BLD_STOREHOUSE
    { 0, 0 },  // BLD_NOTHING9
    { 1, 1 },  // BLD_MILL
    { 1, 2 },  // BLD_BAKERY
    { 1, 2 },  // BLD_SAWMILL
    { 1, 1 },  // BLD_MINT
    { 2, 0 },  // BLD_WELL
    { 1, 1 },  // BLD_SHIPYARD
    { 1, 0 },  // BLD_FARM
    { 1, 2 },  // BLD_DONKEYBREEDER
    { 0, 0 },  // BLD_HARBORBUILDING
};

Building::Building(
        World& buildings,
        BuildingType type,
        State state)
    : world_(buildings),
      pt_(MapPoint::Invalid()),
      type_(type),
      state_(state),
      group_(InvalidProductionGroup),
      captured_(false)
{

}

const MapPoint& Building::GetPt() const
{
    return pt_;
}

MapPoint Building::GetFlag() const
{
    if (pt_.isValid())
        return world_.GetNeighbour(pt_, Direction::SOUTHEAST);
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

unsigned Building::GetGroup() const
{
    return group_;
}

BuildingQuality Building::GetQuality() const
{
    return BUILDING_SIZE[type_];
}

unsigned Building::GetDistance(const MapPoint& pt) const
{
    return world_.CalcDistance(GetPt(), pt);
}

const std::vector<BuildingType>& Building::GetDestTypes(bool& checkGroup) const
{
    const ProductionDest& dest = GOODS_DESTINATIONS[type_];
    checkGroup = dest.checkGroup;
    return dest.types;
}

const Building::TrafficExpected& Building::GetTraffic() const
{
    return GOODS_TRAFFIC[type_];
}

bool Building::IsMilitary() const
{
    return BuildingProperties::IsMilitary(type_);
}

bool Building::IsWarehouse() const
{
    return BuildingProperties::IsWareHouse(type_);
}

unsigned Building::GetGoods(GoodType good) const
{
    const Inventory* inventory = GetInventory();
    if (inventory)
        return inventory->goods[good];
    return 0;
}

unsigned Building::GetJobs(Job job) const
{
    const Inventory* inventory = GetInventory();
    if (inventory)
        return inventory->people[job];
    return 0;
}

bool Building::IsGrouped() const
{
    return type_ == BLD_WOODCUTTER ||
           type_ == BLD_FORESTER ||
           type_ == BLD_SLAUGHTERHOUSE ||
           type_ == BLD_ARMORY ||
           type_ == BLD_IRONSMELTER ||
           type_ == BLD_PIGFARM ||
           type_ == BLD_MILL ||
           type_ == BLD_BAKERY ||
           type_ == BLD_SAWMILL;
}

const Inventory* Building::GetInventory() const
{
    if (!IsWarehouse())
        return nullptr;

    if (state_ != Finished)
        return nullptr;

    const noBaseBuilding* base = world_.GetBaseBuilding(this);

    RTTR_Assert(base);
    RTTR_Assert(base->GetType() == NOP_BUILDING);
    RTTR_Assert(BuildingProperties::IsWareHouse(base->GetBuildingType()));

    const nobBaseWarehouse* warehouse = static_cast<const nobBaseWarehouse*>(base);
    return &warehouse->GetInventory();
}

} // namespace beowulf
