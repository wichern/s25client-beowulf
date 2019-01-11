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
#include "commonDefines.h"

#include <vector>

namespace beowulf {

static const unsigned WOODCUTTER_RADIUS = 6;
static const unsigned FISHER_RADIUS = 7;
static const unsigned FORESTER_RADIUS = 6;
static const unsigned STONEMASON_RADIUS = 8;
static const unsigned FARMER_RADIUS = 2;

struct Production {
    BGoodType good;
    unsigned amount;
};

/*
 * This table shows what type of good a building procudes and how many compared
 * to others in the production chain.
 */
static const Production PRODUCTION[NUM_BUILDING_TYPES] = {
    { BGD_NONE,    0 }, // BLD_HEADQUARTERS
    { BGD_NONE,    0 }, // BLD_BARRACKS
    { BGD_NONE,    0 }, // BLD_GUARDHOUSE
    { BGD_NONE,    0 }, // BLD_NOTHING2
    { BGD_NONE,    0 }, // BLD_WATCHTOWER
    { BGD_NONE,    0 }, // BLD_NOTHING3
    { BGD_NONE,    0 }, // BLD_NOTHING4
    { BGD_NONE,    0 }, // BLD_NOTHING5
    { BGD_NONE,    0 }, // BLD_NOTHING6
    { BGD_NONE,    0 }, // BLD_FORTRESS
    { BGD_STONE,   1 }, // BLD_GRANITEMINE
    { BGD_COAL,    1 }, // BLD_COALMINE
    { BGD_IRONORE, 1 }, // BLD_IRONMINE
    { BGD_GOLD,    1 }, // BLD_GOLDMINE
    { BGD_NONE,    0 }, // BLD_LOOKOUTTOWER
    { BGD_NONE,    0 }, // BLD_NOTHING7
    { BGD_NONE,    0 }, // BLD_CATAPULT
    { BGD_WOOD,    1 }, // BLD_WOODCUTTER
    { BGD_FOOD,    1 }, // BLD_FISHERY
    { BGD_STONE,   1 }, // BLD_QUARRY
    { BGD_TREE,    2 }, // BLD_FORESTER
    { BGD_FOOD,    1 }, // BLD_SLAUGHTERHOUSE
    { BGD_NONE,    0 }, // BLD_HUNTER   // the hunter produces very little food
    { BGD_BEER,    1 }, // BLD_BREWERY
    { BGD_WEAPON,  1 }, // BLD_ARMORY
    { BGD_TOOL,    1 }, // BLD_METALWORKS
    { BGD_IRON,    1 }, // BLD_IRONSMELTER
    { BGD_COAL,    1 }, // BLD_CHARBURNER
    { BGD_PIG,     1 }, // BLD_PIGFARM
    { BGD_NONE,    0 }, // BLD_STOREHOUSE
    { BGD_NONE,    0 }, // BLD_NOTHING9
    { BGD_FLOUR,   1 }, // BLD_MILL
    { BGD_FOOD,    1 }, // BLD_BAKERY
    { BGD_BOARD,   1 }, // BLD_SAWMILL
    { BGD_COIN,    1 }, // BLD_MINT
    { BGD_WATER,   1 }, // BLD_WELL
    { BGD_SHIP,    1 }, // BLD_SHIPYARD
    { BGD_GRAIN,   1 }, // BLD_FARM
    { BGD_DONKEY,  1 }, // BLD_DONKEYBREEDER
    { BGD_NONE,    0 }, // BLD_HARBORBUILDING
};

struct Consumption {
    BGoodType a, b, c;
};


/*
 * This table shows what type of good a building consumes and how many compared
 * to producers.
 */
static const Consumption CONSUMPTION[NUM_BUILDING_TYPES] = {
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_HEADQUARTERS
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_BARRACKS
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_GUARDHOUSE
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING2
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_WATCHTOWER
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING3
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING4
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING5
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING6
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_FORTRESS
    { BGD_FOOD,     BGD_NONE,   BGD_NONE }, // BLD_GRANITEMINE
    { BGD_FOOD,     BGD_NONE,   BGD_NONE }, // BLD_COALMINE
    { BGD_FOOD,     BGD_NONE,   BGD_NONE }, // BLD_IRONMINE
    { BGD_FOOD,     BGD_NONE,   BGD_NONE }, // BLD_GOLDMINE
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_LOOKOUTTOWER
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING7
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_CATAPULT
    { BGD_TREE,     BGD_NONE,   BGD_NONE }, // BLD_WOODCUTTER
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_FISHERY
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_QUARRY
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_FORESTER
    { BGD_PIG,      BGD_NONE,   BGD_NONE }, // BLD_SLAUGHTERHOUSE
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_HUNTER   // the hunter produces very little food
    { BGD_GRAIN,    BGD_WATER,  BGD_NONE }, // BLD_BREWERY
    { BGD_IRON,     BGD_COAL,   BGD_NONE }, // BLD_ARMORY
    { BGD_IRON,     BGD_COAL,   BGD_NONE }, // BLD_METALWORKS
    { BGD_IRONORE,  BGD_COAL,   BGD_NONE }, // BLD_IRONSMELTER
    { BGD_WOOD,     BGD_GRAIN,  BGD_NONE }, // BLD_CHARBURNER
    { BGD_GRAIN,    BGD_WATER,  BGD_NONE }, // BLD_PIGFARM
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_STOREHOUSE
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_NOTHING9
    { BGD_GRAIN,    BGD_NONE,   BGD_NONE }, // BLD_MILL
    { BGD_FLOUR,    BGD_NONE,   BGD_NONE }, // BLD_BAKERY
    { BGD_WOOD,     BGD_WOOD,   BGD_NONE }, // BLD_SAWMILL
    { BGD_COAL,     BGD_GOLD,   BGD_NONE }, // BLD_MINT
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_WELL
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_SHIPYARD
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_FARM
    { BGD_GRAIN,    BGD_WATER,  BGD_NONE }, // BLD_DONKEYBREEDER
    { BGD_NONE,     BGD_NONE,   BGD_NONE }, // BLD_HARBORBUILDING
};

} // namespace beowulf

#endif //! BEOWULF_PRODUCTIONCONSTS_H_INCLUDED
