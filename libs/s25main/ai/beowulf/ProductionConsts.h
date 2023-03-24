// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/Types.h"

#include "commonDefines.h"
#include "gameTypes/BuildingType.h"
#include "gameTypes/GoodTypes.h"
#include "gameData/JobConsts.h"

#include <vector>

namespace beowulf {

static const unsigned WOODCUTTER_RADIUS = 6;
static const unsigned FISHER_RADIUS = 7;
static const unsigned FORESTER_RADIUS = 6;
static const unsigned STONEMASON_RADIUS = 8;
static const unsigned FARMER_RADIUS = 2;

static const unsigned MIN_FISHER_DISTANCE = 8; // 8 is a wild guess.
static const unsigned UPPER_TRAFFIC_LIMIT = 30;

struct ProductionStats
{
    BGoodType production;
    std::vector<BGoodType> consumption;
    int speed; // higher number is faster (consumption and production)
};

static const helpers::EnumArray<ProductionStats, BuildingType> PRODUCTION = {{
  {BGD_NONE, {}, 0},                        // BuildingType::Headquarters
  {BGD_NONE, {}, 0},                        // BuildingType::Barracks
  {BGD_NONE, {}, 0},                        // BuildingType::Guardhouse
  {BGD_NONE, {}, 0},                        // BLD_NOTHING2
  {BGD_NONE, {}, 0},                        // BuildingType::Watchtower
  {BGD_NONE, {}, 0},                        // BLD_NOTHING3
  {BGD_NONE, {}, 0},                        // BLD_NOTHING4
  {BGD_NONE, {}, 0},                        // BLD_NOTHING5
  {BGD_NONE, {}, 0},                        // BLD_NOTHING6
  {BGD_NONE, {}, 0},                        // BuildingType::Fortress
  {BGD_STONE, {BGD_FOOD}, 5},               // BuildingType::GraniteMine
  {BGD_COAL, {BGD_FOOD}, 5},                // BuildingType::CoalMine
  {BGD_IRONORE, {BGD_FOOD}, 5},             // BuildingType::IronMine
  {BGD_GOLD, {BGD_FOOD}, 5},                // BuildingType::GoldMine
  {BGD_NONE, {}, 0},                        // BuildingType::LookoutTower
  {BGD_NONE, {}, 0},                        // BLD_NOTHING7
  {BGD_NONE, {}, 0},                        // BuildingType::Catapult
  {BGD_WOOD, {}, 10},                       // BuildingType::Woodcutter
  {BGD_FOOD, {}, 5},                        // BuildingType::Fishery
  {BGD_STONE, {}, 10},                      // BuildingType::Quarry
  {BGD_NONE, {}, 0},                        // BuildingType::Forester
  {BGD_FOOD, {BGD_PIG}, 10},                // BuildingType::Slaughterhouse
  {BGD_FOOD, {}, 3},                        // BuildingType::Hunter
  {BGD_BEER, {BGD_GRAIN, BGD_WATER}, 10},   // BuildingType::Brewery
  {BGD_WEAPON, {BGD_IRON, BGD_COAL}, 10},   // BuildingType::Armory
  {BGD_TOOL, {BGD_IRON, BGD_BOARD}, 10},    // BuildingType::Metalworks
  {BGD_IRON, {BGD_IRONORE, BGD_COAL}, 10},  // BuildingType::Ironsmelter
  {BGD_COAL, {BGD_BOARD}, 10},              // BuildingType::Charburner
  {BGD_PIG, {BGD_GRAIN, BGD_WATER}, 10},    // BuildingType::PigFarm
  {BGD_NONE, {}, 0},                        // BuildingType::Storehouse
  {BGD_NONE, {}, 0},                        // BLD_NOTHING9
  {BGD_FLOUR, {BGD_GRAIN}, 10},             // BuildingType::Mill
  {BGD_FOOD, {BGD_FLOUR, BGD_WATER}, 10},   // BuildingType::Bakery
  {BGD_BOARD, {BGD_WOOD}, 20},              // BuildingType::Sawmill
  {BGD_COIN, {BGD_GOLD, BGD_COAL}, 10},     // BuildingType::Mint
  {BGD_WATER, {}, 50},                      // BuildingType::Well
  {BGD_SHIP, {BGD_BOARD}, 2},               // BuildingType::Shipyard
  {BGD_GRAIN, {}, 5},                       // BuildingType::Farm
  {BGD_DONKEY, {BGD_GRAIN, BGD_WATER}, 10}, // BuildingType::DonkeyBreeder
  {BGD_NONE, {}, 0}                         // BuildingType::HarborBuilding
}};

static const std::string SUPPRESS_UNUSED BGOOD_NAMES[BGD_COUNT] = {
  "Weapon", "Beer", "Tool",  "Ship", "Donkey", "Coin", "Iron",  "Coal",  "Ironore", "Gold",
  "Board",  "Wood", "Stone", "Food", "Flour",  "Pig",  "Grain", "Water", "<none>"};

static const helpers::EnumArray<BResourceType, BuildingType> REQUIRED_RESOURCES = {
  BResourceCount,           // BuildingType::Headquarters
  BResourceCount,           // BuildingType::Barracks
  BResourceCount,           // BuildingType::Guardhouse
  BResourceCount,           // BLD_NOTHING2
  BResourceCount,           // BuildingType::Watchtower
  BResourceCount,           // BLD_NOTHING3
  BResourceCount,           // BLD_NOTHING4
  BResourceCount,           // BLD_NOTHING5
  BResourceCount,           // BLD_NOTHING6
  BResourceCount,           // BuildingType::Fortress
  BResourceGranite,         // BuildingType::GraniteMine
  BResourceCoal,            // BuildingType::CoalMine
  BResourceIron,            // BuildingType::IronMine
  BResourceGold,            // BuildingType::GoldMine
  BResourceCount,           // BuildingType::LookoutTower
  BResourceCount,           // BLD_NOTHING7
  BResourceCount,           // BuildingType::Catapult
  BResourceCount,           // BuildingType::Woodcutter
  BResourceFish,            // BuildingType::Fishery
  BResourceStone,           // BuildingType::Quarry
  BResourcePlantSpace_6,    // BuildingType::Forester
  BResourceCount,           // BuildingType::Slaughterhouse
  BResourceHuntableAnimals, // BuildingType::Hunter
  BResourceCount,           // BuildingType::Brewery
  BResourceCount,           // BuildingType::Armory
  BResourceCount,           // BuildingType::Metalworks
  BResourceCount,           // BuildingType::Ironsmelter
  BResourcePlantSpace_2,    // BuildingType::Charburner
  BResourceCount,           // BuildingType::PigFarm
  BResourceCount,           // BuildingType::Storehouse
  BResourceCount,           // BLD_NOTHING9
  BResourceCount,           // BuildingType::Mill
  BResourceCount,           // BuildingType::Bakery
  BResourceCount,           // BuildingType::Sawmill
  BResourceCount,           // BuildingType::Mint
  BResourceWater,           // BuildingType::Well
  BResourceCount,           // BuildingType::Shipyard
  BResourcePlantSpace_2,    // BuildingType::Farm
  BResourceCount,           // BuildingType::DonkeyBreeder
  BResourceCount,           // BuildingType::HarborBuilding
};

} // namespace beowulf
