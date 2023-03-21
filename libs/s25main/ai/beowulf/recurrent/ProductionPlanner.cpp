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

#include "ai/beowulf/recurrent/ProductionPlanner.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Heuristics.h"

#include "gameData/BuildingConsts.h"
#include "gameData/BuildingProperties.h"
#include "gameData/JobConsts.h"
#include "buildings/noBuildingSite.h"
#include "helpers/containerUtils.h"

#include <boost/foreach.hpp>

#include <algorithm>

namespace beowulf {

/*
 * We need to have a production plan for every road network.
 * To not waste time computing the connection of different road networks we make these assumptions:
 *
 * - There is one network for the HQ.
 * - There is one network for every harbour not connected to the HQ.
 *
 * Only the HQ will try to build soldiers.
 * Every other network will try to provide resources to the HQ (ores, stone, wood, food).
 */

ProductionPlanner::Region::Region()
{
    resources.fill(0);
}

ProductionPlanner::Region::Region(
        const Building* building,
        const MapPoint& flag,
        bool main)
    : isMain(main),
      regionFlag(flag),
      buildings{ building }
{
    resources.fill(0);
}

unsigned ProductionPlanner::Region::GetTotalJobs(Job job) const
{
    unsigned ret = 0;
    for (const Building* building : buildings) {
        if (building->IsWarehouse())
            ret += building->GetJobs(job);
    }
    return ret;
}

unsigned ProductionPlanner::Region::GetTotalGoods(GoodType good) const
{
    unsigned ret = 0;
    for (const Building* building : buildings) {
        if (building->IsWarehouse())
            ret += building->GetGoods(good);
    }
    return ret;
}

unsigned ProductionPlanner::Region::CountBuildings(
        const std::vector<BuildingType>&& types,
        bool constructionFinished) const
{
    return static_cast<unsigned>(std::count_if(buildings.begin(), buildings.end(),
                                               [&](const Building* building)
    {
        if (constructionFinished && building->GetState() != Building::Finished)
            return false;
        return helpers::contains(types, building->GetType());
    }));
}

ProductionPlanner::ProductionPlanner(Beowulf* beowulf)
    : RecurrentBase(beowulf, 15, 0) // Plan only every 15*16 gf.
{
}

void ProductionPlanner::OnRun()
{
    CalculateRegions();

    for (auto& it : regions_) {
        const MapPoint regionPt = it.first;
        Region& region = it.second;
        Plan(regionPt, region);
    }
}

unsigned ProductionPlanner::GetTotalProduction(BGoodType type) const
{
    unsigned ret = 0;
    for (auto& it : regions_) {
        const Region& region = it.second;
        ret += static_cast<unsigned>(region.production[type].produced);
    }
    return ret;
}

void ProductionPlanner::Plan(const MapPoint& regionPt, Region& region)
{
    const unsigned builder = 10;
    auto constructionSites = std::count_if(region.buildings.begin(), region.buildings.end(),
                                           [](const Building* bld) { return bld->GetState() != Building::Finished; });
    if (builder <= constructionSites + beowulf_->build.GetRequestCount())
        return;
    unsigned maxRequests = builder - static_cast<unsigned>(constructionSites) - beowulf_->build.GetRequestCount();

    // EXISTING GROUPS
    // Go through all known production groups and check if any of them still misses a building.
    if (0 == beowulf_->build.GetRequestCount()) {
        for (const World::ProductionGroup& group : beowulf_->world.groups) {
            // Check for non existend buildings before checking if group is part of this region
            // for performance reason.
            for (size_t i = 0; i < group.types.size(); ++i) {
                if (!group.buildings[i]) {
                    // Check if the group is part of this region.
                    if (!beowulf_->world.CanConnectBuilding(group.region, region.regionFlag, false))
                        break;
                    RequestBuilding(regionPt, group.types[i], group.id);
                    if (0 == --maxRequests)
                        return;
                }
            }
        }
    }

    // BGD_BOARD
    // We extend board production based on the number of military buildings.
    // We use a fibonacci scale based on military building count.
    unsigned sawmillsPlaced = region.CountBuildings({ BuildingType::Sawmill });
    unsigned sawmills = sawmillsPlaced + beowulf_->build.GetRequestCount({ BuildingType::Sawmill }, regionPt);
    unsigned militaries = region.CountBuildings({ BuildingType::Barracks, BuildingType::Guardhouse, BuildingType::Watchtower, BuildingType::Fortress }, true);
    static const std::vector<unsigned> fib = { 1, 2, 3, 5, 8, 13, 21, 34 };
    if (sawmills < 2 || militaries > fib[sawmills]) {
        World::ProductionGroup& group = beowulf_->world.CreateGroup({ BuildingType::Woodcutter, BuildingType::Woodcutter, BuildingType::Forester, BuildingType::Sawmill }, regionPt);
        RequestBuilding(regionPt, BuildingType::Woodcutter, group.id);
        RequestBuilding(regionPt, BuildingType::Woodcutter, group.id);
        RequestBuilding(regionPt, BuildingType::Forester, group.id);
        RequestBuilding(regionPt, BuildingType::Sawmill, group.id);
        maxRequests -= std::min(maxRequests, static_cast<unsigned>(group.types.size()));

        if (0 == maxRequests)
            return;
    }

    // Do not allow construction of anything else until we successfully placed at least one sawmill!
    if (0 == sawmillsPlaced)
        return;

    // BGD_STONE
    // We extend board production based on the number of military buildings.
    // We use a fibonacci scale based on military building count.
    unsigned stoneProducer = region.CountBuildings({ BuildingType::Quarry, BuildingType::GraniteMine });
    stoneProducer += beowulf_->build.GetRequestCount({ BuildingType::Quarry, BuildingType::GraniteMine }, regionPt);
    if (stoneProducer < 1 || militaries > fib[stoneProducer]) {
        auto quarries =  region.CountBuildings({ BuildingType::Quarry });
        if (quarries == 0 && region.resources[BResourceStone] > 0) {
            RequestBuilding(regionPt, BuildingType::Quarry);
            maxRequests--;
        } else if (region.resources[BResourceStone] >= 2 * quarries) {
            RequestBuilding(regionPt, BuildingType::Quarry);
            maxRequests--;
        } else if (region.resources[BResourceGranite] > 0) {
            RequestBuilding(regionPt, BuildingType::GraniteMine);
            maxRequests--;
        }

        if (0 == maxRequests)
            return;
    }

    // BGD_BEER
    // Produce more beer if more is needed.
    if (region.production[BGD_BEER].consumed > region.production[BGD_BEER].produced) {
        RequestBuilding(regionPt, BuildingType::Brewery);
        if (0 == --maxRequests)
            return;
    }

    // BGD_TOOL
    // Create a metalworks if we produce iron and do not yet have a metalworks.
    if (region.isMain) {
        if (beowulf_->world.GetBuildings(BuildingType::Metalworks).empty() && region.production[BGD_IRON].produced > 0) {
            RequestBuilding(regionPt, BuildingType::Metalworks);
            if (0 == --maxRequests)
                return;
        }
    }

    // BGD_COIN
    if (region.isMain) {
        if (region.production[BGD_GOLD].produced > region.production[BGD_GOLD].consumed) {
            RequestBuilding(regionPt, BuildingType::Mint);
            if (0 == --maxRequests)
                return;
        }
    }

    // BGD_WEAPON
    // Create an armory, ironsmelter group if we produce enough coal and ironore.
    if (region.isMain) {
        if (region.production[BGD_COAL].produced > region.production[BGD_COAL].consumed
                && region.production[BGD_IRONORE].produced > region.production[BGD_IRONORE].consumed)
        {
            World::ProductionGroup& group = beowulf_->world.CreateGroup({ BuildingType::Ironsmelter, BuildingType::Armory }, regionPt);
            RequestBuilding(regionPt, BuildingType::Ironsmelter, group.id);
            RequestBuilding(regionPt, BuildingType::Armory, group.id);
            maxRequests -= std::min(maxRequests, static_cast<unsigned>(group.types.size()));

            if (0 == maxRequests)
                return;
        }
    }

    // BGD_FOOD
    if (region.production[BGD_GRAIN].produced > region.production[BGD_GRAIN].consumed) {
        if (0 == beowulf_->build.GetRequestCount({ BuildingType::Bakery, BuildingType::Slaughterhouse }, regionPt)) {
            // Make only mills until we are out of bakers and then make only pigfarms until we are out of butchers.
            // Then continue with mills.

            unsigned possibleBakers = region.GetTotalJobs(JOB_BAKER) + region.GetTotalGoods(*JOB_CONSTS[JOB_BAKER].tool);
            unsigned possibleButchers = region.GetTotalJobs(JOB_BUTCHER) + region.GetTotalGoods(*JOB_CONSTS[JOB_BUTCHER].tool);
            if (possibleBakers > 0 || 0 == possibleButchers) {
                World::ProductionGroup& group = beowulf_->world.CreateGroup({ BuildingType::Mill, BuildingType::Bakery }, regionPt);
                RequestBuilding(regionPt, BuildingType::Mill, group.id);
                RequestBuilding(regionPt, BuildingType::Bakery, group.id);
                maxRequests -= std::min(maxRequests, static_cast<unsigned>(group.types.size()));
            } else {
                World::ProductionGroup& group = beowulf_->world.CreateGroup({ BuildingType::Slaughterhouse, BuildingType::PigFarm }, regionPt);
                RequestBuilding(regionPt, BuildingType::Slaughterhouse, group.id);
                RequestBuilding(regionPt, BuildingType::PigFarm, group.id);
                maxRequests -= std::min(maxRequests, static_cast<unsigned>(group.types.size()));
            }
        }
    }

    // BGD_DONKEY
    // Only build one donkeybreeder and not before all donkeys left the storages.
    if (region.isMain && 0 == region.GetTotalJobs(JOB_PACKDONKEY)) {
        unsigned donkeyBreeder = region.CountBuildings({ BuildingType::DonkeyBreeder });
        donkeyBreeder += beowulf_->build.GetRequestCount({ BuildingType::DonkeyBreeder }, regionPt);

        if (0 == region.CountBuildings({ BuildingType::DonkeyBreeder })) {
            RequestBuilding(regionPt, BuildingType::Mint);
            if (0 == --maxRequests)
                return;
        }
    }

    // Hunter
    // We add hunters based on the amount of deer available.
    if (beowulf_->metalworks.JobOrToolOrQueueSpace(JOB_HUNTER)) {
        if (0 == beowulf_->build.GetRequestCount({ BuildingType::Hunter }, regionPt)) {
            if (region.resources[BResourceHuntableAnimals] > 0) {
                if (0 == beowulf_->build.GetRequestCount({ BuildingType::Hunter }, regionPt)) {
                    RequestBuilding(regionPt, BuildingType::Hunter);
                    if (0 == --maxRequests)
                        return;
                }
            }
        }
    }

    // Fishermen
    // We add fishermen based on the amount of fish available.
    if (beowulf_->metalworks.JobOrToolOrQueueSpace(JOB_FISHER)) {
        if (0 == beowulf_->build.GetRequestCount({ BuildingType::Fishery }, regionPt)) {
            if (region.resources[BResourceHuntableAnimals] > 0) {
                if (0 == beowulf_->build.GetRequestCount({ BuildingType::Fishery }, regionPt)) {
                    RequestBuilding(regionPt, BuildingType::Fishery);
                    if (0 == --maxRequests)
                        return;
                }
            }
        }
    }

    // Farms, Wells
    // try to produce at least one more than required.
    for (BuildingType type : { BuildingType::Farm, BuildingType::Well }) {
        BResourceType required = REQUIRED_RESOURCES[type];
        // Has resources in region?
        if (0 == region.resources[required])
            continue;

        // Check whether we can create a worker for this building.
        if (!beowulf_->metalworks.JobOrToolOrQueueSpace(*BLD_WORK_DESC[type].job))
            continue;

        // Already requested?
        if (beowulf_->build.GetRequestCount({ type }, regionPt) > 0)
            continue;

        BGoodType good = PRODUCTION[type].production;
        if (region.production[good].produced <= region.production[good].consumed + PRODUCTION[type].speed) {
            RequestBuilding(regionPt, type);
            if (0 == --maxRequests)
                return;
        }
    }

    // Mines
    // We prefer to build the mine for which we have the least overproduction.
    int coalOverproduction = static_cast<int>(region.production[BGD_COAL].produced) - static_cast<int>(region.production[BGD_COAL].consumed);
    int ironOverproduction = static_cast<int>(region.production[BGD_IRON].produced) - static_cast<int>(region.production[BGD_IRON].consumed);
    int goldOverproduction = static_cast<int>(region.production[BGD_GOLD].produced) - static_cast<int>(region.production[BGD_GOLD].consumed);

    unsigned requestedCoal = beowulf_->build.GetRequestCount({ BuildingType::CoalMine }, regionPt);
    unsigned requestedIron = beowulf_->build.GetRequestCount({ BuildingType::IronMine }, regionPt);
    unsigned requestedGold = beowulf_->build.GetRequestCount({ BuildingType::GoldMine }, regionPt);

    if (coalOverproduction < ironOverproduction && coalOverproduction < goldOverproduction && region.resources[BResourceCoal] && 0 == requestedCoal) {
        RequestBuilding(regionPt, BuildingType::CoalMine);
        if (0 == --maxRequests)
            return;
    } else if (ironOverproduction < coalOverproduction && ironOverproduction < goldOverproduction && region.resources[BResourceIron] && 0 == requestedIron) {
        RequestBuilding(regionPt, BuildingType::IronMine);
        if (0 == --maxRequests)
            return;
    } else if (goldOverproduction < coalOverproduction && goldOverproduction < ironOverproduction && region.resources[BResourceGold] && 0 == requestedGold) {
        RequestBuilding(regionPt, BuildingType::GoldMine);
        if (0 == --maxRequests)
            return;
    } else {
        // Build anything we can.
        if (region.resources[BResourceCoal] > 0 && 0 == requestedCoal) {
            RequestBuilding(regionPt, BuildingType::CoalMine);
            if (0 == --maxRequests)
                return;
        }
        if (region.resources[BResourceIron] > 0 && 0 == requestedIron) {
            RequestBuilding(regionPt, BuildingType::IronMine);
            if (0 == --maxRequests)
                return;
        }
        if (region.resources[BResourceGold] > 0 && 0 == requestedGold) {
            RequestBuilding(regionPt, BuildingType::GoldMine);
            if (0 == --maxRequests)
                return;
        }
    }
}

void ProductionPlanner::RequestBuilding(
        const MapPoint& regionPt,
        BuildingType type,
        unsigned group)
{
    if (beowulf_->metalworks.JobOrToolOrQueueSpace(*BLD_WORK_DESC[type].job, true)) {
        Building* bld = beowulf_->world.Create(type, Building::PlanningRequest, group);
        beowulf_->build.Request(bld, regionPt);
    } else {
        RTTR_Assert(false); // we previously checked that we have the job/good or a metalworks.
    }
}

int ProductionPlanner::GetOvercapacity(const ProductionPlanner::Region &region, BuildingType type, BGoodType good) const
{
    // Some production has to happen locally, other can be globally.
    switch (type) {
    case BuildingType::Bakery:
    case BuildingType::Mill:
    case BuildingType::DonkeyBreeder:
    case BuildingType::PigFarm:
    {
        return region.production[good].produced - region.production[good].consumed;
    }
    case BuildingType::CoalMine:
    case BuildingType::IronMine:
    case BuildingType::GoldMine:
    case BuildingType::Mint:
    case BuildingType::Ironsmelter:
    case BuildingType::Armory:
    {
        return globalProduction_[good].produced - globalProduction_[good].consumed;
    }
    default:
        return 0;
    }
}

void ProductionPlanner::CalculateRegions()
{
    regions_.clear();

    const World& world = beowulf_->world;
    std::set<const Building*> considered;

    Building* hq = world.GetHQ();
    if (hq) {
        considered.insert(hq);
        regions_[hq->GetFlag()] = Region(hq, hq->GetFlag(), true);
        CalculateRegion(hq->GetFlag(), considered);
    }

    for (const Building* bld : world.GetBuildings()) {
        if (bld->GetType() == BuildingType::HarborBuilding) {
            if (considered.find(bld) != considered.end())
                continue;
            if (considered.insert(bld).second) {
                regions_[bld->GetFlag()] = Region(bld, bld->GetFlag());
                CalculateRegion(hq->GetFlag(), considered);
            }
        }
    }

    // If there was no more HQ, make the firs region with industry the main region.
    if (nullptr == hq) {
        bool found = false;
        for (auto& it : regions_) {
            Region& region = it.second;
            for (const Building* bld : region.buildings) {
                if (bld->GetType() == BuildingType::Metalworks || bld->GetType() == BuildingType::Armory || bld->GetType() == BuildingType::Ironsmelter) {
                    region.isMain = true;
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }

        if (!found) {
            // Make first region main.
            auto it = regions_.begin();
            Region& region = (*it).second;
            region.isMain = true;
        }
    }
}

void ProductionPlanner::CalculateRegion(
        const MapPoint& flag,
        std::set<const Building*>& considered)
{
    World& world = beowulf_->world;
    Region& region = regions_[flag];

    // Find all buildings.
    FloodFill(world, flag,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        return world.HasRoad(pt, dir);
    },
    // Action
    [&](const MapPoint& pt)
    {
        if (world.HasFlag(pt)) {
            MapPoint buildingPt = world.GetNeighbour(pt, Direction::NorthWest);
            const Building* bld = world.GetBuilding(buildingPt);
            if (bld) {
                if (considered.find(bld) != considered.end())
                    return;

                region.buildings.push_back(bld);
                considered.insert(bld);

                const ProductionStats& prod = PRODUCTION[bld->GetType()];
                if (prod.production != BGD_NONE) {
                    region.production[prod.production].produced += prod.speed;
                    globalProduction_[prod.production].produced += prod.speed;
                }
                for (BGoodType type : prod.consumption) {
                    region.production[type].consumed += prod.speed;
                    globalProduction_[type].consumed += prod.speed;
                }
            }
        }
    });

    // Find all resources.
    BuildLocations bl(world, false);
    bl.Calculate(flag);

    for (const MapPoint& pt : bl.Get()) {
        BuildingQuality bq = bl.Get(pt);

        if (bq == BuildingQuality::Mine) {
            region.resources[BResourceCoal] += world.resources.GetReachable(pt, BResourceCoal, false, true, false);
            region.resources[BResourceIron] += world.resources.GetReachable(pt, BResourceIron, false, true, false);
            region.resources[BResourceGold] += world.resources.GetReachable(pt, BResourceGold, false, true, false);
            region.resources[BResourceGranite] += world.resources.GetReachable(pt, BResourceGranite, false, true, false);
        } else if (bq >= BuildingQuality::Hut && bq <= BuildingQuality::Castle) {
            region.resources[BResourceWater] += world.resources.GetReachable(pt, BResourceWater, false);
            region.resources[BResourcePlantSpace_6] += world.resources.GetReachable(pt, BResourcePlantSpace_6, false);
            region.resources[BResourceFish] += world.resources.GetReachable(pt, BResourceFish, false);
            region.resources[BResourceHuntableAnimals] += world.resources.GetReachable(pt, BResourceHuntableAnimals, false);
            region.resources[BResourceWood] += world.resources.GetReachable(pt, BResourceWood, false, false);
            region.resources[BResourceStone] += world.resources.GetReachable(pt, BResourceStone, false, false);
        }
        if (bq == BuildingQuality::Castle) {
            region.resources[BResourcePlantSpace_2] += world.resources.GetReachable(pt, BResourcePlantSpace_2);
        }
    }
}

}  // namespace beowulf
