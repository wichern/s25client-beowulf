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

#include "ai/beowulf/ExpansionPlanner.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/BuildLocations.h"

#include "ai/AIInterface.h"
#include "buildings/noBuildingSite.h"
#include "gameData/BuildingProperties.h"
#include "gameData/MilitaryConsts.h"
#include "gameData/BuildingConsts.h"

namespace beowulf {

ExpansionPlanner::ExpansionPlanner(
        AIInterface& aii,
        World& world)
    : aii_(aii),
      world_(world)
{

}

void ExpansionPlanner::Update(rnet_id_t rnet, std::vector<Building*>& requests)
{
    RTTR_Assert(rnet != InvalidRoadNetwork);

    // Do not check on every GF.
    if (lastCheck_ != checkInterval_) {
        lastCheck_++;
        return;
    }
    lastCheck_ = 0;

    // Check whether we actually want to expand.
    if (!ShouldExpand())
        return;

    // Use a very simple approach of getting all build locations and use
    // the one that provides the most new territory.
    //
    // Also only build guard houses for now.
    BuildLocations locations(world_);
    locations.Calculate(world_, world_.GetFlag(rnet));

    unsigned bestArea = 10; // minimum new area to get
    MapPoint bestPoint = MapPoint::Invalid();

    BuildingType type = BLD_GUARDHOUSE;
    for (MapPoint pt : locations.Get(BUILDING_SIZE[type])) {
        unsigned area = 0;
        world_.VisitPointsInRadius(pt, MILITARY_RADIUS[1], [&area, this](const MapPoint& pt)
        {
            if (!aii_.gwb.IsPlayerTerritory(pt, aii_.GetPlayerId()) && !aii_.gwb.IsSeaPoint(pt))
            {
                area++;
            }
        }, false);

        if (area > bestArea) {
            bestArea = area;
            bestPoint = pt;
        }
    }

    if (bestPoint.isValid()) {
        Building* building = world_.Create(type, Building::PlanningRequest, InvalidProductionGroup, bestPoint);
        requests.push_back(building);
    }
}

bool ExpansionPlanner::ShouldExpand() const
{
    // @todo: Use StoragePlanner or similar for this task.
    const auto& inventory = aii_.GetInventory();
    unsigned soldiers = inventory[JOB_PRIVATE] + inventory[JOB_PRIVATEFIRSTCLASS] + inventory[JOB_SERGEANT] + inventory[JOB_OFFICER] + inventory[JOB_GENERAL];
    if (soldiers < minSoldiers_)
        return false;

    const std::list<noBuildingSite*>& sites = aii_.GetBuildingSites();
    auto mililtaryBuildingSites = std::count_if(sites.begin(), sites.end(), [](const noBuildingSite* site)
    { return BuildingProperties::IsMilitary(site->GetBuildingType()); });
    if (mililtaryBuildingSites > maxParallelSites_)
        return false;

    // @todo: Get amount of territory we want to claim.
    return true;
}

} // namespace beowulf
