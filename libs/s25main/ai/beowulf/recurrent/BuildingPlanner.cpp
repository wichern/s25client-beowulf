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

#include "ai/beowulf/recurrent/BuildingPlanner.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Debug.h"

#include "ai/AIInterface.h"
#include "gameData/BuildingProperties.h"
#include "notifications/BuildingNote.h"

#include <iostream>
#include "gameData/BuildingConsts.h"

namespace beowulf {

BuildingPlanner::BuildingPlanner(Beowulf* beowulf)
    : RecurrentBase(beowulf, 1, 0),
      costs_(beowulf->GetAII(), beowulf->world)
{
    // Change build order so that sawmills have highest priority.
//    BuildOrders order = beowulf_->player.GetStandardBuildOrder();
//    size_t sawmillPos = 0;
//    while (order[sawmillPos] != BuildingType::Sawmill) sawmillPos++;
//    BuildingType oldTop = order[0];
//    order[0] = BuildingType::Sawmill;
//    order[sawmillPos] = oldTop;
//    beowulf_->GetAII().ChangeBuildOrder(true, order);
}

BuildingPlanner::~BuildingPlanner()
{
}

void BuildingPlanner::OnRun()
{
    if (current_.requests.empty()) {
        for (const auto& reqs : requests_) {
            const std::vector<Building*>& vec = reqs.second;
            if (!vec.empty()) {
                current_.dest = reqs.first;
                current_.requests = vec;
                current_.searches = 0;
                requests_.erase(reqs.first);
                break;
            }
        }
    }

    if (!current_.requests.empty()) {
        if (current_.searches < 1) {
            Search();
            Execute();
        }
    }
}

void BuildingPlanner::Request(Building* building, const MapPoint& regionPt)
{
    // Heuristic cannot evaluate military building positions.
    RTTR_Assert(!BuildingProperties::IsMilitary(building->GetType()) || building->GetPt().isValid());
    requests_[regionPt].push_back(building);

    //std::cout << "Request(" << BUILDING_NAMES[building->GetType()] << ")" << std::endl;
}

unsigned BuildingPlanner::GetRequestCount() const
{
    unsigned ret = static_cast<unsigned>(current_.requests.size());
    for (const auto& req : requests_)
        ret += req.second.size();
    return ret;
}

unsigned BuildingPlanner::GetRequestCount(
        const std::vector<BuildingType>&& types,
        const MapPoint& regionPt) const
{
    unsigned ret = 0;

    for (const auto& req : requests_) {
        const std::vector<Building*>& buildings = req.second;
        ret += std::count_if(buildings.begin(), buildings.end(),
                             [&](const Building* bld)
        {
            return std::find(types.begin(), types.end(), bld->GetType()) != types.end()
                    && beowulf_->world.CanConnectBuilding(regionPt, req.first, false);
        });
    }

    if (!beowulf_->world.CanConnectBuilding(regionPt, current_.dest, true))
        return ret;

    ret += std::count_if(current_.requests.begin(), current_.requests.end(),
                         [types](const Building* bld)
    {
        return std::find(types.begin(), types.end(), bld->GetType()) != types.end() ;
    });

    return ret;
}

void BuildingPlanner::Search()
{
    /*
     * Our strategy is just to sort the buildings in a clever order
     * and then find the best position one by one.
     */

    static const unsigned c_buildingOrder[helpers::MaxEnumValue_v<BuildingType>] =
    {
        0,  // BuildingType::Headquarters
        5,  // BuildingType::Barracks
        4,  // BuildingType::Guardhouse
        0,  // BLD_NOTHING2
        3,  // BuildingType::Watchtower
        0,  // BLD_NOTHING3
        0,  // BLD_NOTHING4
        0,  // BLD_NOTHING5
        0,  // BLD_NOTHING6
        2,  // BuildingType::Fortress
        30, // BuildingType::GraniteMine
        30, // BuildingType::CoalMine
        30, // BuildingType::IronMine
        30, // BuildingType::GoldMine
        100,// BuildingType::LookoutTower
        0,  // BLD_NOTHING7
        50, // BuildingType::Catapult
        30, // BuildingType::Woodcutter
        20, // BuildingType::Fishery
        20, // BuildingType::Quarry
        10, // BuildingType::Forester
        10, // BuildingType::Slaughterhouse
        20, // BuildingType::Hunter
        10, // BuildingType::Brewery
        10, // BuildingType::Armory
        10, // BuildingType::Metalworks
        20, // BuildingType::Ironsmelter
        30, // BuildingType::Charburner
        20, // BuildingType::PigFarm
        2,  // BuildingType::Storehouse
        0,  // BLD_NOTHING9
        20, // BuildingType::Mill
        10, // BuildingType::Bakery
        40, // BuildingType::Sawmill
        10, // BuildingType::Mint
        100,// BuildingType::Well
        1,  // BuildingType::Shipyard
        30, // BuildingType::Farm
        15, // BuildingType::DonkeyBreeder
        1,  // BuildingType::HarborBuilding
    };
    std::stable_sort(current_.requests.begin(), current_.requests.end(),
              [&](const Building* l, const Building* r)
    {
        return c_buildingOrder[l->GetType()] < c_buildingOrder[r->GetType()];
    });
    std::stable_sort(current_.requests.begin(), current_.requests.end(),
              [&](const Building* l, const Building* r)
    {
        if (l->GetPt().isValid() && !r->GetPt().isValid())
            return true;
        if (!l->GetPt().isValid() && r->GetPt().isValid())
            return false;
        if (l->IsGrouped() && r->IsGrouped()) {
            if (l->GetGroup() < r->GetGroup())
                return true;
            if (l->GetGroup() > r->GetGroup())
                return false;
        }
        return false;
    });

    current_.searches++;
}

void BuildingPlanner::Execute()
{
    BuildLocations locations(beowulf_->world, false);
    locations.Calculate(current_.dest);
    for (Building* building : current_.requests) {
        Place(building, locations);
    }
    current_.requests.clear();
}

double HyperVolume(const std::vector<double>& vec);
double HyperVolume(const std::vector<double>& vec)
{
    double ret = 1.0;
    for (double v : vec)
        ret *= v;
    return ret;
}

bool BuildingPlanner::Place(
        Building* building,
        BuildLocations& locations)
{
    // Is this building type already known to not be placeable at the moment?
    if (blacklist_[building->GetType()])
        return false;

    MapPoint pt;

    if (building->GetPt().isValid()) {
        if (!canUseBq(locations.Get(building->GetPt()), building->GetQuality())) {
            std::cout << "No viable position found for a " << BUILDING_NAMES[building->GetType()] << " (no build locations)" << std::endl;
            blacklist_[building->GetType()] = true;
            return false;
        }
        pt = building->GetPt();
    } else {
        if (!FindBestPosition(building, pt, HyperVolume, locations)) {
            std::cout << "No viable position found for a " << BUILDING_NAMES[building->GetType()] << " (no location with positive score)" << std::endl;
            blacklist_[building->GetType()] = true;
            return false;
        }
    }

    beowulf_->world.Construct(building, pt);
    locations.Update(pt, building->GetQuality() >= BuildingQuality::Castle ? 4 : 3);
    if (!beowulf_->roads.Connect(building, &locations)) {
        RTTR_Assert(false);
        beowulf_->world.Deconstruct(building);
    }

    return true;
}

void BuildingPlanner::OnBuildingNote(const BuildingNote& note)
{
    if (!enabled_)
        return;

    Building* bld = beowulf_->world.GetBuilding(note.pos);
    RTTR_Assert(!bld || bld->GetType() == note.bld);

    // Lua request to build this building.
    if (note.type == BuildingNote::LuaOrder) {
        bld = beowulf_->world.Create(note.bld, Building::PlanningRequest, InvalidProductionGroup, note.pos);
        Request(bld, beowulf_->world.GetHQFlag());
    }
}

void BuildingPlanner::OnNodeNote(const NodeNote& note)
{
    (void)note;
    blacklist_.reset();
}

} // namespace beowulf
