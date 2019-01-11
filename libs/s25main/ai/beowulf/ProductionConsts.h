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
#ifndef BEOWULF_PRODUCTIONCONSTS_H_INCLUDED
#define BEOWULF_PRODUCTIONCONSTS_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "gameTypes/GoodTypes.h"
#include "gameTypes/BuildingType.h"
#include "gameData/JobConsts.h"
#include "commonDefines.h"

#include <vector>

namespace beowulf {

static const unsigned WOODCUTTER_RADIUS = 6;
static const unsigned FISHER_RADIUS = 7;
static const unsigned FORESTER_RADIUS = 6;
static const unsigned STONEMASON_RADIUS = 8;
static const unsigned FARMER_RADIUS = 2;

static const unsigned MIN_FISHER_DISTANCE = 8; // 8 is a wild guess.
static const unsigned UPPER_TRAFFIC_LIMIT = 30;

struct ProductionStats {
    BGoodType production;
    std::vector<BGoodType> consumption;
    int speed; // higher number is faster (consumption and production)
};

static const ProductionStats PRODUCTION[NUM_BUILDING_TYPES] = {
   { BGD_NONE      , { }                        , 0  },  // BLD_HEADQUARTERS
   { BGD_NONE      , { }                        , 0  },  // BLD_BARRACKS
   { BGD_NONE      , { }                        , 0  },  // BLD_GUARDHOUSE
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING2
   { BGD_NONE      , { }                        , 0  },  // BLD_WATCHTOWER
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING3
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING4
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING5
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING6
   { BGD_NONE      , { }                        , 0  },  // BLD_FORTRESS
   { BGD_STONE     , { BGD_FOOD }               , 5  },  // BLD_GRANITEMINE
   { BGD_COAL      , { BGD_FOOD }               , 5  },  // BLD_COALMINE
   { BGD_IRONORE   , { BGD_FOOD }               , 5  },  // BLD_IRONMINE
   { BGD_GOLD      , { BGD_FOOD }               , 5  },  // BLD_GOLDMINE
   { BGD_NONE      , { }                        , 0  },  // BLD_LOOKOUTTOWER
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING7
   { BGD_NONE      , { }                        , 0  },  // BLD_CATAPULT
   { BGD_WOOD      , { }                        , 10 },  // BLD_WOODCUTTER
   { BGD_FOOD      , { }                        , 5  },  // BLD_FISHERY
   { BGD_STONE     , { }                        , 10 },  // BLD_QUARRY
   { BGD_NONE      , { }                        , 0  },  // BLD_FORESTER
   { BGD_FOOD      , { BGD_PIG  }               , 10 },  // BLD_SLAUGHTERHOUSE
   { BGD_FOOD      , { }                        , 3  },  // BLD_HUNTER
   { BGD_BEER      , { BGD_GRAIN, BGD_WATER }   , 10 },  // BLD_BREWERY
   { BGD_WEAPON    , { BGD_IRON, BGD_COAL }     , 10 },  // BLD_ARMORY
   { BGD_TOOL      , { BGD_IRON, BGD_BOARD }    , 10 },  // BLD_METALWORKS
   { BGD_IRON      , { BGD_IRONORE, BGD_COAL}   , 10 },  // BLD_IRONSMELTER
   { BGD_COAL      , { BGD_BOARD }              , 10 },  // BLD_CHARBURNER
   { BGD_PIG       , { BGD_GRAIN, BGD_WATER }   , 10 },  // BLD_PIGFARM
   { BGD_NONE      , { }                        , 0  },  // BLD_STOREHOUSE
   { BGD_NONE      , { }                        , 0  },  // BLD_NOTHING9
   { BGD_FLOUR     , { BGD_GRAIN }              , 10 },  // BLD_MILL
   { BGD_FOOD      , { BGD_FLOUR, BGD_WATER }   , 10 },  // BLD_BAKERY
   { BGD_BOARD     , { BGD_WOOD }               , 20 },  // BLD_SAWMILL
   { BGD_COIN      , { BGD_GOLD, BGD_COAL }     , 10 },  // BLD_MINT
   { BGD_WATER     , { }                        , 50 },  // BLD_WELL
   { BGD_SHIP      , { BGD_BOARD }              , 2  },  // BLD_SHIPYARD
   { BGD_GRAIN     , { }                        , 5  },  // BLD_FARM
   { BGD_DONKEY    , { BGD_GRAIN, BGD_WATER }   , 10 },  // BLD_DONKEYBREEDER
   { BGD_NONE      , { }                        , 0  },  // BLD_HARBORBUILDING
};


static const std::string SUPPRESS_UNUSED BGOOD_NAMES[BGD_COUNT] = {
    "Weapon",
    "Beer",
    "Tool",
    "Ship",
    "Donkey",
    "Coin",
    "Iron",
    "Coal",
    "Ironore",
    "Gold",
    "Board",
    "Wood",
    "Stone",
    "Food",
    "Flour",
    "Pig",
    "Grain",
    "Water",
    "<none>"
};

static const BResourceType REQUIRED_RESOURCES[NUM_BUILDING_TYPES] = {
   BResourceCount           , // BLD_HEADQUARTERS
   BResourceCount           , // BLD_BARRACKS
   BResourceCount           , // BLD_GUARDHOUSE
   BResourceCount           , // BLD_NOTHING2
   BResourceCount           , // BLD_WATCHTOWER
   BResourceCount           , // BLD_NOTHING3
   BResourceCount           , // BLD_NOTHING4
   BResourceCount           , // BLD_NOTHING5
   BResourceCount           , // BLD_NOTHING6
   BResourceCount           , // BLD_FORTRESS
   BResourceGranite         , // BLD_GRANITEMINE
   BResourceCoal            , // BLD_COALMINE
   BResourceIron            , // BLD_IRONMINE
   BResourceGold            , // BLD_GOLDMINE
   BResourceCount           , // BLD_LOOKOUTTOWER
   BResourceCount           , // BLD_NOTHING7
   BResourceCount           , // BLD_CATAPULT
   BResourceCount           , // BLD_WOODCUTTER
   BResourceFish            , // BLD_FISHERY
   BResourceStone           , // BLD_QUARRY
   BResourcePlantSpace_6    , // BLD_FORESTER
   BResourceCount           , // BLD_SLAUGHTERHOUSE
   BResourceHuntableAnimals , // BLD_HUNTER
   BResourceCount           , // BLD_BREWERY
   BResourceCount           , // BLD_ARMORY
   BResourceCount           , // BLD_METALWORKS
   BResourceCount           , // BLD_IRONSMELTER
   BResourcePlantSpace_2    , // BLD_CHARBURNER
   BResourceCount           , // BLD_PIGFARM
   BResourceCount           , // BLD_STOREHOUSE
   BResourceCount           , // BLD_NOTHING9
   BResourceCount           , // BLD_MILL
   BResourceCount           , // BLD_BAKERY
   BResourceCount           , // BLD_SAWMILL
   BResourceCount           , // BLD_MINT
   BResourceWater           , // BLD_WELL
   BResourceCount           , // BLD_SHIPYARD
   BResourcePlantSpace_2    , // BLD_FARM
   BResourceCount           , // BLD_DONKEYBREEDER
   BResourceCount           , // BLD_HARBORBUILDING
};

} // namespace beowulf

#endif //! BEOWULF_PRODUCTIONCONSTS_H_INCLUDED
