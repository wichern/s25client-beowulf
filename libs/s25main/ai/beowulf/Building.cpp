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

static const helpers::EnumArray<ProductionDest, BuildingType> SUPPRESS_UNUSED GOODS_DESTINATIONS =
{{
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Headquarters
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Barracks
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Guardhouse
    { false, {  } },  // BLD_NOTHING2
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Watchtower
    { false, {  } },  // BLD_NOTHING3
    { false, {  } },  // BLD_NOTHING4
    { false, {  } },  // BLD_NOTHING5
    { false, {  } },  // BLD_NOTHING6
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Fortress
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::GraniteMine
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::CoalMine
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::IronMine
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::GoldMine
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::LookoutTower
    { false, {  } },  // BLD_NOTHING7
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Catapult
    { true,  { BuildingType::Sawmill } },       // BuildingType::Woodcutter
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Fishery
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Quarry
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Forester
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Slaughterhouse
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Hunter   // the hunter produces very little food
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Brewery
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Armory
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Metalworks
    { true,  { BuildingType::Armory, BuildingType::Metalworks } }, // BuildingType::Ironsmelter
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Charburner
    { true,  { BuildingType::Slaughterhouse } },// BuildingType::PigFarm
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::Storehouse
    { false, {  } },  // BLD_NOTHING9
    { true,  { BuildingType::Bakery } },        // BuildingType::Mill
    { true,  { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Bakery
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Sawmill
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Mint
    { true,  { BuildingType::Bakery, BuildingType::Brewery, BuildingType::DonkeyBreeder, BuildingType::Slaughterhouse } }, // BuildingType::Well
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::Shipyard
    { true,  { BuildingType::Mill, BuildingType::Brewery, BuildingType::DonkeyBreeder, BuildingType::Slaughterhouse } }, // BuildingType::Farm
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },    // BuildingType::DonkeyBreeder
    { false, { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding } },  // BuildingType::HarborBuilding
}};

static const SUPPRESS_UNUSED helpers::EnumArray<Building::TrafficExpected, BuildingType> SUPPRESS_UNUSED GOODS_TRAFFIC =
{{
    { 0, 0 },  // BuildingType::Headquarters
    { 0, 0 },  // BuildingType::Barracks
    { 0, 0 },  // BuildingType::Guardhouse
    { 0, 0 },  // BLD_NOTHING2
    { 0, 0 },  // BuildingType::Watchtower
    { 0, 0 },  // BLD_NOTHING3
    { 0, 0 },  // BLD_NOTHING4
    { 0, 0 },  // BLD_NOTHING5
    { 0, 0 },  // BLD_NOTHING6
    { 0, 0 },  // BuildingType::Fortress
    { 1, 1 },  // BuildingType::GraniteMine
    { 1, 1 },  // BuildingType::CoalMine
    { 1, 1 },  // BuildingType::IronMine
    { 1, 1 },  // BuildingType::GoldMine
    { 0, 0 },  // BuildingType::LookoutTower
    { 0, 0 },  // BLD_NOTHING7
    { 0, 0 },  // BuildingType::Catapult
    { 0, 0 },  // BuildingType::Woodcutter
    { 1, 0 },  // BuildingType::Fishery
    { 2, 0 },  // BuildingType::Quarry
    { 0, 0 },  // BuildingType::Forester
    { 1, 1 },  // BuildingType::Slaughterhouse
    { 1, 0 },  // BuildingType::Hunter   // the hunter produces very little food
    { 1, 2 },  // BuildingType::Brewery
    { 1, 2 },  // BuildingType::Armory
    { 1, 2 },  // BuildingType::Metalworks
    { 1, 2 },  // BuildingType::Ironsmelter
    { 1, 2 },  // BuildingType::Charburner
    { 1, 2 },  // BuildingType::PigFarm
    { 0, 0 },  // BuildingType::Storehouse
    { 0, 0 },  // BLD_NOTHING9
    { 1, 1 },  // BuildingType::Mill
    { 1, 2 },  // BuildingType::Bakery
    { 1, 2 },  // BuildingType::Sawmill
    { 1, 1 },  // BuildingType::Mint
    { 2, 0 },  // BuildingType::Well
    { 1, 1 },  // BuildingType::Shipyard
    { 1, 0 },  // BuildingType::Farm
    { 1, 2 },  // BuildingType::DonkeyBreeder
    { 0, 0 },  // BuildingType::HarborBuilding
}};

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
        return world_.GetNeighbour(pt_, Direction::SouthEast);
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
    return type_ == BuildingType::Woodcutter ||
           type_ == BuildingType::Forester ||
           type_ == BuildingType::Slaughterhouse ||
           type_ == BuildingType::Armory ||
           type_ == BuildingType::Ironsmelter ||
           type_ == BuildingType::PigFarm ||
           type_ == BuildingType::Mill ||
           type_ == BuildingType::Bakery ||
           type_ == BuildingType::Sawmill;
}

const Inventory* Building::GetInventory() const
{
    if (!IsWarehouse())
        return nullptr;

    if (state_ != Finished)
        return nullptr;

    const noBaseBuilding* base = world_.GetBaseBuilding(this);

    RTTR_Assert(base);
    RTTR_Assert(base->GetType() == NodalObjectType::Building);
    RTTR_Assert(BuildingProperties::IsWareHouse(base->GetBuildingType()));

    const nobBaseWarehouse* warehouse = static_cast<const nobBaseWarehouse*>(base);
    return &warehouse->GetInventory();
}

} // namespace beowulf
