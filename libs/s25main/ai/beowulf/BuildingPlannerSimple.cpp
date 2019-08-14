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

#include "ai/beowulf/BuildingPlannerSimple.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/ResourceMap.h"

#include "gameData/BuildingConsts.h"

#include "ai/AIInterface.h"

#include <limits>
#include <cmath> /* exp() */
#include <algorithm> /* std::min */

namespace beowulf {

BuildingPlannerSimple::BuildingPlannerSimple(
        AIInterface& aii,
        Buildings& buildings,
        ResourceMap& resources,
        rnet_id_t rnet)
    : BuildingPlannerBase(aii, buildings, resources, rnet)
{

}

BuildingPlannerSimple::~BuildingPlannerSimple()
{

}

void BuildingPlannerSimple::Init(const std::vector<Building*>& requests)
{
    requests_ = requests;
}

void BuildingPlannerSimple::Search()
{
    static const unsigned c_buildingOrder[NUM_BUILDING_TYPES] =
    {
        0,  // BLD_HEADQUARTERS
        5,  // BLD_BARRACKS
        4,  // BLD_GUARDHOUSE
        0,  // BLD_NOTHING2
        3,  // BLD_WATCHTOWER
        0,  // BLD_NOTHING3
        0,  // BLD_NOTHING4
        0,  // BLD_NOTHING5
        0,  // BLD_NOTHING6
        2,  // BLD_FORTRESS
        30, // BLD_GRANITEMINE
        30, // BLD_COALMINE
        30, // BLD_IRONMINE
        30, // BLD_GOLDMINE
        100,// BLD_LOOKOUTTOWER
        0,  // BLD_NOTHING7
        50, // BLD_CATAPULT
        30, // BLD_WOODCUTTER
        20, // BLD_FISHERY
        20, // BLD_QUARRY
        30, // BLD_FORESTER
        10, // BLD_SLAUGHTERHOUSE
        20, // BLD_HUNTER
        10, // BLD_BREWERY
        10, // BLD_ARMORY
        10, // BLD_METALWORKS
        20, // BLD_IRONSMELTER
        30, // BLD_CHARBURNER
        20, // BLD_PIGFARM
        2,  // BLD_STOREHOUSE
        0,  // BLD_NOTHING9
        20, // BLD_MILL
        10, // BLD_BAKERY
        10, // BLD_SAWMILL
        10, // BLD_MINT
        100,// BLD_WELL
        1,  // BLD_SHIPYARD
        30, // BLD_FARM
        15, // BLD_DONKEYBREEDER
        1,  // BLD_HARBORBUILDING
    };
    std::stable_sort(requests_.begin(), requests_.end(),
              [this](const Building* l, const Building* r)
    {
        return c_buildingOrder[l->GetType()] < c_buildingOrder[r->GetType()];
    });
    std::stable_sort(requests_.begin(), requests_.end(),
              [this](const Building* l, const Building* r)
    {
        if (l->GetPt().isValid() && !r->GetPt().isValid())
            return true;
        if (!l->GetPt().isValid() && r->GetPt().isValid())
            return true;
        if (l->IsProduction() && r->IsProduction()) {
            if (l->GetGroup() < r->GetGroup())
                return true;
            if (l->GetGroup() > r->GetGroup())
                return false;
        }
        return false;
    });

    searches_++;
}

void BuildingPlannerSimple::Execute()
{
    for (Building* building : requests_) {
        MapPoint pt;
        std::vector<Direction> route;
        Place(building, pt, route, true);
    }
}

unsigned BuildingPlannerSimple::GetSearches() const
{
    return searches_;
}

unsigned BuildingPlannerSimple::GetMaxSearches() const
{
    return 1;
}

} // namespace beowulf
