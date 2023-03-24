// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIResource.h"
#include "commonDefines.h"
#include "gameTypes/BuildingType.h"
#include "gameTypes/Direction.h"
#include "gameTypes/MapCoordinates.h"

#include <boost/scoped_array.hpp>
#include <string>

namespace beowulf {

static constexpr unsigned InvalidProductionGroup = std::numeric_limits<unsigned>::max();

enum FlagState
{
    // There is no flag at this position.
    FlagDoesNotExist,

    // A game command for placing this flag has been sent with AIInterface.
    FlagRequested,

    // The flag/road is exists.
    FlagFinished,

    FlagDestructionRequested,
};

enum RoadState
{
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
enum BGoodType
{
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

    BGD_STONE,

    BGD_FOOD,
    BGD_FLOUR,
    BGD_PIG,
    BGD_GRAIN,

    BGD_WATER,

    BGD_NONE,
    BGD_COUNT
};

enum BResourceType
{
    BResourceIron = 0,
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
