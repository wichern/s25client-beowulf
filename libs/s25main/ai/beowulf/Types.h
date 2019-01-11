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
#ifndef BEOWULF_TYPES_H_INCLUDED
#define BEOWULF_TYPES_H_INCLUDED

#include "gameTypes/BuildingType.h"
#include "gameTypes/MapCoordinates.h"
#include "gameTypes/Direction.h"
#include "commonDefines.h"
#include "ai/AIResource.h"

#include <string>
#include <boost/scoped_array.hpp>

namespace beowulf {

typedef unsigned    ProductionGroup;
typedef unsigned    Island;

static constexpr Island InvalidProductionGroup = std::numeric_limits<ProductionGroup>::max();
static constexpr Island InvalidIsland = std::numeric_limits<Island>::max();

enum FlagState {
    // There is no flag at this position.
    FlagDoesNotExist,

    // A game command for placing this flag has been sent with AIInterface.
    FlagRequested,

    // The flag/road is exists.
    FlagFinished,

    FlagDestructionRequested,
};

enum RoadState {
    // There is no road at this position.
    RoadDoesNotExist,

    // A game command for placing this road has been sent with AIInterface.
    RoadRequested,

    // The road is exists.
    RoadFinished,

    RoadDestructionRequested,
};

/*
 * BGoodType contains a list of goods, but reduces them for ProductionPlanner
 * usage. E.g. BGD_FOOD allows requesting food and deciding how to produce it
 * later.
 *
 * The goods are sorted such that production of a good never depends on
 * a good with a smaller index.
 */
enum BGoodType {
    BGD_WEAPON = 0,
    BGD_BEER,

    BGD_TOOL,

    BGD_SHIP,
    BGD_DONKEY,

    BGD_COIN,
    BGD_IRON,

    BGD_COAL,
    BGD_IRONORE,
    BGD_GOLD,

    BGD_BOARD,
    BGD_WOOD,
    BGD_TREE,

    BGD_STONE,

    BGD_FOOD,
    BGD_FLOUR,
    BGD_PIG,
    BGD_GRAIN,

    BGD_WATER,

    BGD_NONE,
    BGD_COUNT
};

enum BResourceType {
    BResourceIron,
    BResourceGold,
    BResourceCoal,
    BResourceGranite,
    BResourceWater,
    BResourcePlantSpace_2, // PlantSpace in radius 2 (Farmer)
    BResourcePlantSpace_6, // PlantSpace in radius 6 (Forester)
    BResourceFish,
    BResourceHuntableAnimals,
    BResourceWood,
    BResourceStone,
    BResourceCount
};

} // namespace beowulf

#endif //! BEOWULF_TYPES_H_INCLUDED
