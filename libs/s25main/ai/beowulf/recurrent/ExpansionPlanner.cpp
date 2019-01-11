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

#include "ai/beowulf/recurrent/ExpansionPlanner.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Helper.h"

#include "ai/AIInterface.h"
#include "buildings/noBuildingSite.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobMilitary.h"
#include "gameData/BuildingProperties.h"
#include "gameData/MilitaryConsts.h"
#include "gameData/BuildingConsts.h"
#include "helpers/containerUtils.h"

namespace beowulf {

/*
 * Expanding well is hard!
 *
 * There is multitude of things to consider:
 *   - Save stone
 *   - Save soldiers
 *   - Secure infrastructure
 *   - Secure narrow map spots
 *   - Attack enemy
 *   - Reach resources and farmland
 *   - Avoid catapults
 *   - FOW
 *
 * Evaluating possible new locations we also have to consider that some territory
 * is already being reached by military buildings currently under construction.
 *
 * The importance of different goals depends on higher goals:
 *   - Resources already available
 *   - Agression level
 *   - Availability of stones
 */

ExpansionPlanner::ExpansionPlanner(Beowulf* beowulf)
    : RecurrentBase(beowulf, 10, 0) // check every 10*16 gf
{

}

void ExpansionPlanner::OnRun()
{
    // @todo: Try the following loop that deconstructs unused military buildings.
//    for (const nobMilitary* building : beowulf_->GetAII().GetMilitaryBuildings()) {
//        if (building->IsUseless()) {
//            beowulf_->world.Deconstruct(beowulf_->world.GetBuilding(building->GetPos()));
//        }
//    }

    // Check whether we actually want to expand.
    if (!ShouldExpand())
        return;

    // Get list of expandable territories.
    std::vector<MapPoint> starts;
    for (nobBaseWarehouse* warehouse : beowulf_->GetAII().GetStorehouses()) {
        // Check for enough soldiers, builders and materials.
        const Inventory& inventory = warehouse->GetInventory();
        unsigned soldiers = inventory[JOB_PRIVATE] + inventory[JOB_PRIVATEFIRSTCLASS] + inventory[JOB_SERGEANT] + inventory[JOB_OFFICER] + inventory[JOB_GENERAL];
        if (soldiers < minSoldiers_)
            continue;
        if (inventory[JOB_BUILDER] < 1 || inventory[JOB_PLANER] < 1)
            continue;
        const BuildingCost& cost = BUILDING_COSTS[beowulf_->GetAII().GetNation()][BLD_GUARDHOUSE];
        if (inventory[GD_BOARDS] < cost.boards || inventory[GD_STONES] < cost.stones)
            continue;

        // Check that this warehouse is not connected to another warehouse.
        // (We only want to add one military building for every region).
        bool skip = false;
        for (const MapPoint& start : starts) {
            if (!beowulf_->world.GetPath(warehouse->GetFlagPos(), start).empty()) {
                skip = true;
                break;
            }
        }
        if (skip)
            continue;

        // @todo: Keep track of which region is currently building a new military building.

        starts.push_back(warehouse->GetFlagPos());
    }

    for (const MapPoint& start : starts)
        Expand(start);
}

void ExpansionPlanner::Expand(const MapPoint& pt)
{
    /*
     * # For every possible military location:
     * #   quality = GuardHouse
     * #   if enemy catapult in reach that will not be destroyed by this building:
     * #     skip
     * #   if enemy close:
     * #     quality = Tower
     * #   if would destroy friendly buildings
     * #     skip
     * #   if isStrategicPosition(pt):
     * #     quality = Fortress
     * #   for pt in radius(GuardHouse) not yet in owned territory:
     * #     for type in resource_types:
     * #       res[type] = known (by geologist) + assumed (close to known, or random on mountain, or less random if close to mountain or even less random if close to something else.)
     * #   rating = 0
     * #   for type in resource_types:
     * #     rating += res[type] * value(type) // value can be adjusted if resources are low
     * #
     * # choose best rated location
     */

    // Get all possible military locations and search for the best rated position.
    BuildLocations locations(beowulf_->world, false);
    locations.Calculate(pt);

    unsigned bestRating = 0;
    MapPoint bestPoint = MapPoint::Invalid();
    BuildingType bestType = BLD_NOTHING;

    std::vector<MapPoint> additionalTerritory;
    std::vector<const noBaseBuilding*> destroyed;

    World& world = beowulf_->world;

    for (const MapPoint& loc : locations.Get(BQ_HUT)) {
        BuildingType type = BLD_GUARDHOUSE;

        if (!world.CanBuildMilitary(loc))
            continue;
        if (world.IsRestricted(loc))
            continue;

        // check if the enemy is close:
        unsigned enemies = world.GetEnemySoldiersInReach(pt);
        if (enemies > 1) {
            TryImprove(type, locations.Get(loc));
            if (enemies > 5) {
                TryImprove(type, locations.Get(loc));
            }
        } else {
            if (world.IsEnemyNear(pt, MAX_MILITARY_DISTANCE_NEAR))
                TryImprove(type, locations.Get(loc));
        }

        world.PredictExpansionResults(loc, type, additionalTerritory, destroyed);

        std::vector<const noBaseBuilding*> catapults = world.GetEnemyCatapultsInReach(pt);
        bool catapultsRemaining = false;
        for (const noBaseBuilding* catapult : catapults) {
            if (!helpers::contains(destroyed, catapult)) {
                catapultsRemaining = true;
                break;
            }
        }
        if (catapultsRemaining)
            continue;

        // Check what resources we can gain with this building.
        unsigned ores = 0;
        unsigned stones = 0;
        unsigned plantspace = 0;
        for (const MapPoint& p : additionalTerritory) {
            ores +=   world.resources.Get(p, BResourceCoal, true)
                    + world.resources.Get(p, BResourceIron, true)
                    + world.resources.Get(p, BResourceGold, true);
            stones += world.resources.Get(p, BResourceStone, true)
                    + world.resources.Get(p, BResourceGranite, true);
            plantspace += world.resources.Get(p, BResourcePlantSpace_2, true);
        }

        // @todo: Weigth resources based on what we need the most.
        unsigned rating = (ores * 2u) + stones + plantspace + (static_cast<unsigned>(destroyed.size()) * 2u);
        if (rating > bestRating) {
            bestRating = rating;
            bestPoint = loc;
            bestType = type;
        }
    }

    if (bestPoint.isValid()) {
        Building* building = world.Create(bestType, Building::PlanningRequest, InvalidProductionGroup, bestPoint);
        beowulf_->build.Request(building, pt);
    }
}

bool ExpansionPlanner::ShouldExpand() const
{
    unsigned mililtaryBuildingSites = 0;    // no more than maxParallelSites_ military building sites.
    unsigned sawmill = 0;                   // at least a sawmill, a woodcutter and a quarry
    unsigned woodcutter = 0;
    unsigned quarry = 0;
    for (const Building* bld : beowulf_->world.GetBuildings()) {
        switch (bld->GetType()) {
        case BLD_SAWMILL:
            sawmill++;
            break;
        case BLD_WOODCUTTER:
            woodcutter++;
            break;
        case BLD_QUARRY:
            quarry++;
            break;
        default:
        {
            if (BuildingProperties::IsMilitary(bld->GetType())) {
                if (bld->GetState() == Building::UnderConstruction) {
                    mililtaryBuildingSites++;
                    if (mililtaryBuildingSites >= maxParallelSites_) {
                        return false;
                    }
                }
            }
        } break;
        }
    }

    // @todo: Get amount of territory we want to claim.
    return sawmill > 0 && woodcutter > 0 && quarry > 0;
}

bool ExpansionPlanner::TryImprove(BuildingType& type, BuildingQuality bq) const
{
    if (type == BLD_BARRACKS && beowulf_->GetAII().CanBuildBuildingtype(BLD_GUARDHOUSE)) {
        type = BLD_GUARDHOUSE;
        return true;
    }
    if (type == BLD_GUARDHOUSE && beowulf_->GetAII().CanBuildBuildingtype(BLD_WATCHTOWER) && bq >= BQ_HOUSE) {
        type = BLD_WATCHTOWER;
        return true;
    }
    if (type == BLD_WATCHTOWER && beowulf_->GetAII().CanBuildBuildingtype(BLD_FORTRESS) && bq >= BQ_CASTLE) {
        type = BLD_FORTRESS;
        return true;
    }
    return false;
}

} // namespace beowulf
