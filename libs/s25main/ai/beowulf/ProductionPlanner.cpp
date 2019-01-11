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

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/ProductionPlanner.h"
#include "ai/beowulf/ProductionConsts.h"

#include "libutil/FileWriter.h"
#include "ai/AIInterface.h"
#include "gameData/BuildingConsts.h"
#include "libutil/Log.h"

#include <boost/foreach.hpp>

namespace beowulf {

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
    "Tree",
    "Stone",
    "Food",
    "Flour",
    "Pig",
    "Grain",
    "Water",
    "<none>"
};

ProductionPlanner::ProductionPlanner(Beowulf& beowulf)
    : beowulf_(beowulf)
{
    memset(goal_, 0, sizeof(goal_));
    memset(production_, 0, sizeof(production_));
    memset(required_, 0, sizeof(required_));
}

ProductionPlanner::~ProductionPlanner()
{

}

bool ProductionPlanner::Plan()
{
//    // Calculate the current production.
//    memset(production_, 0, sizeof(production_));
//    memset(required_, 0, sizeof(required_));

//    for (unsigned i = 0; i < NUM_BUILDING_TYPES; ++i)
//        BOOST_FOREACH(const Building* building, buildingPlanner_.GetBuildings())
//            AddBuilding(building->type);

//    std::vector<BGoodType> increase;
//    IncreaseProductionHeuristic heuristic(beowulf_);
//    heuristic.Evaluate(increase);
//    bool ret = !increase.empty();
//    while (!increase.empty()) {
//        IncreaseProductionGoal(increase.back());
//        increase.pop_back();
//    }

//    // Fix current production chain and expand with goals.
//    for (unsigned  i = 0; i < BGD_COUNT; ++i) {
//        while (required_[i] > production_[i])
//            ExtendProduction((BGoodType)i);
//        while (goal_[i] > production_[i])
//            ExtendProduction((BGoodType)i);
//    }

    return false;
}

void ProductionPlanner::Debug(const std::string& path) const
{
    FileWriter writer(path);
    writer.writeText(" Good         Goals          Current\n", 0);
    writer.writeText("-----------------------------------------\n", 0);

    char buffer[64];

    for (unsigned i = 0; i < BGD_COUNT; ++i) {
        snprintf(buffer, 64, "%*s", 8, BGOOD_NAMES[i].c_str());
        writer.writeText(buffer, 0);
        snprintf(buffer, 64, "   | %*d", 8, goal_[i]);
        writer.writeText(buffer, 0);
        snprintf(buffer, 64, "   | %*d\n", 10, production_[i]);
        writer.writeText(buffer, 0);
    }
}

void ProductionPlanner::IncreaseProductionGoal(BGoodType type)
{
    LOG.write("[BEOWULF][ProductionPlanner] Requested increase in %s production.\n") % BGOOD_NAMES[type];
    goal_[type]++;
}

void ProductionPlanner::AddBuilding(BuildingType type)
{
    const Production& p = PRODUCTION[type];
    if (p.good != BGD_NONE)
        production_[p.good] += p.amount;

    const Consumption& c = CONSUMPTION[type];
    if (c.a != BGD_NONE)
        required_[c.a]++;
    if (c.b != BGD_NONE)
        required_[c.b]++;
    if (c.c != BGD_NONE)
        required_[c.c]++;
}

void ProductionPlanner::ExtendProduction(BGoodType type)
{
    BuildingType building = BLD_NOTHING;

    switch (type) {
    case BGD_WEAPON:
    {
        building = BLD_ARMORY;
    } break;
    case BGD_BEER:
    {
        building = BLD_BREWERY;
    } break;
    case BGD_TOOL:
    {
        building = BLD_METALWORKS;
    } break;
    case BGD_SHIP:
    {
        building = BLD_SHIPYARD;
    } break;
    case BGD_DONKEY:
    {
        building = BLD_DONKEYBREEDER;
    } break;
    case BGD_COIN:
    {
        building = BLD_MILL;
    } break;
    case BGD_IRON:
    {
        building = BLD_IRONSMELTER;
    } break;
    case BGD_COAL:
    {
        building = BLD_COALMINE;
    } break;
    case BGD_IRONORE:
    {
        building = BLD_IRONMINE;
    } break;
    case BGD_GOLD:
    {
        building = BLD_GOLDMINE;
    } break;
    case BGD_BOARD:
    {
        building = BLD_SAWMILL;
    } break;
    case BGD_WOOD:
    {
        building = BLD_WOODCUTTER;
    } break;
    case BGD_TREE:
    {
        building = BLD_FORESTER;
    } break;
    case BGD_STONE:
    {
        // @todo: granitemine would also be possible
        building = BLD_QUARRY;
    } break;
    case BGD_FOOD:
    {
        /*
         * @todo: depending on tools, available land and fish we decide which food
         * source we want.
         */
        building = BLD_BAKERY;
    } break;
    case BGD_FLOUR:
    {
        building = BLD_MILL;
    } break;
    case BGD_PIG:
    {
        building = BLD_PIGFARM;
    } break;
    case BGD_GRAIN:
    {
        building = BLD_FARM;
    } break;
    case BGD_WATER:
    {
        building = BLD_WELL;
    } break;
    default:
    {
        RTTR_Assert(false);
    } break;
    }

    // @todo
    //buildingPlanner_.Request(building);
    AddBuilding(building);
}

} // namespace beowulf
