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

#include "ai/beowulf/Heuristics.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/ProductionConsts.h"

#include "ai/AIInterface.h"
#include "world/GameWorldBase.h"
#include "buildings/nobBaseWarehouse.h"
#include "gameData/BuildingConsts.h"
#include "gameData/BuildingProperties.h"
#include "gameData/JobConsts.h"
#include "gameTypes/Inventory.h"

namespace beowulf {

struct BuildingLocationChecks {
    // How much resources we nead to have the least.
    unsigned requiredResources;

    // A group member to which the distance is scored.
    std::vector<BuildingType> groupMemberDistances;

    // Whether distance to the destination of produced wares is scored.
    bool waresDestination;
};

// Map building types to the resource it needs to find on resource map.
static const BuildingLocationChecks S_build_location_checks[NUM_BUILDING_TYPES] = {
   { 0, {},                     false }, // BLD_HEADQUARTERS
   { 0, {},                     false }, // BLD_BARRACKS
   { 0, {},                     false }, // BLD_GUARDHOUSE
   { 0, {},                     false }, // BLD_NOTHING2
   { 0, {},                     false }, // BLD_WATCHTOWER
   { 0, {},                     false }, // BLD_NOTHING3
   { 0, {},                     false }, // BLD_NOTHING4
   { 0, {},                     false }, // BLD_NOTHING5
   { 0, {},                     false }, // BLD_NOTHING6
   { 0, {},                     false }, // BLD_FORTRESS
   { 1, {},                     true  }, // BLD_GRANITEMINE
   { 1, {},                     true  }, // BLD_COALMINE
   { 1, {},                     true  }, // BLD_IRONMINE
   { 1, {},                     true  }, // BLD_GOLDMINE
   { 0, {},                     false }, // BLD_LOOKOUTTOWER
   { 0, {},                     false }, // BLD_NOTHING7
   { 0, {},                     false }, // BLD_CATAPULT
   { 0, { BLD_FORESTER, BLD_WOODCUTTER, BLD_SAWMILL }, true  }, // BLD_WOODCUTTER
   { 1, {},                     true  }, // BLD_FISHERY
   { 1, {},                     true  }, // BLD_QUARRY
   { 4, { BLD_WOODCUTTER },     true  }, // BLD_FORESTER
   { 0, { BLD_PIGFARM },        true  }, // BLD_SLAUGHTERHOUSE
   { 2, {},                     false }, // BLD_HUNTER
   { 0, {},                     true  }, // BLD_BREWERY
   { 0, { BLD_IRONSMELTER },    true  }, // BLD_ARMORY
   { 0, {},                     true  }, // BLD_METALWORKS
   { 0, {},                     true  }, // BLD_IRONSMELTER
   { 8, {},                     true  }, // BLD_CHARBURNER
   { 0, {},                     true  }, // BLD_PIGFARM
   { 0, {},                     false }, // BLD_STOREHOUSE
   { 0, {},                     false }, // BLD_NOTHING9
   { 0, {},                     true  }, // BLD_MILL
   { 0, { BLD_MILL },           true  }, // BLD_BAKERY
   { 0, { BLD_WOODCUTTER },     true  }, // BLD_SAWMILL
   { 0, {},                     true  }, // BLD_MINT
   { 1, {},                     true  }, // BLD_WELL
   { 0, {},                     false }, // BLD_SHIPYARD
   {20, {},                     true  }, // BLD_FARM
   { 0, {},                     false }, // BLD_DONKEYBREEDER
   { 0, {},                     false }, // BLD_HARBORBUILDING
};

BuildingPositionCosts::BuildingPositionCosts(const AIInterface& aii,
                                             World& world)
    : aii_(aii),
      world_(world)
{

}

// High is better
double RateHigh(unsigned val, const std::vector<unsigned>& groups);
double RateHigh(unsigned val, const std::vector<unsigned>& groups)
{
    unsigned count = 0;
    while (val > groups[count] && count < groups.size()) count++;
    return 1.0 / (static_cast<double>(groups.size() + 2) - static_cast<double>(count));
}

// Smaller is better
double RateSmall(unsigned val, const std::vector<unsigned>& groups);
double RateSmall(unsigned val, const std::vector<unsigned>& groups)
{
    return 1.0 - RateHigh(val, groups);
}

bool BuildingPositionCosts::Score(
        std::vector<double>& score,
        const Building* building,
        const MapPoint& pt)
{
    RTTR_Assert(!BuildingProperties::IsMilitary(building->GetType()));

    // Set of known storage building types.
    static const std::vector<BuildingType> c_storages =
    { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING };

    // Distance rating thresholds.
    static const std::vector<unsigned> c_distanceGroups =
    { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };

    // Resouces available rating thresholds.
    static const std::vector<unsigned> c_resourceGroups =
    { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100 };

    const BuildingLocationChecks& checks = S_build_location_checks[building->GetType()];
    if (checks.requiredResources > 0) {
        BResourceType resourceType = REQUIRED_RESOURCES[building->GetType()];
        unsigned resources = world_.resources.GetReachable(pt, resourceType);

        // Only valid if enough resources are available.
        if (resources < checks.requiredResources)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(RateHigh(resources, c_resourceGroups));
    }

    // Should be placed close to group members.
    for (BuildingType memberType : checks.groupMemberDistances) {
        auto bld = world_.GroupMemberDistance(pt, building->GetGroup(), memberType);
        score.push_back(RateSmall(bld.second, c_distanceGroups));
    }

    if (checks.waresDestination) {
        // Should be placed close to destination of its wares.
        Building* dst = world_.GetGoodsDest(building, pt);
        if (dst)
            score.push_back(RateSmall(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(1.0);
    }

    // Should not be close to farms or charburners.
    if (world_.GetNearestBuilding(pt, { BLD_FARM, BLD_CHARBURNER }, building).second < (2*FARMER_RADIUS))
        return false;
    else
        score.push_back(1.0);

    switch (building->GetType()) {
    case BLD_LOOKOUTTOWER:
    {
        // Should have a lot of undiscovered area around.
        int undiscovered = 0;
        aii_.gwb.VisitPointsInRadius(pt, VISUALRANGE_LOOKOUTTOWER,
                                     [&](const MapPoint& pt)
        { if (!aii_.IsVisible(pt)) undiscovered++; }, false);

        if (undiscovered == 0)
            return false;

        // Should have enough distance to other towers.
        auto bld = world_.GetNearestBuilding(pt, { BLD_LOOKOUTTOWER });
        if (bld.first.isValid() && bld.second < (VISUALRANGE_LOOKOUTTOWER/3)) {
            score.push_back(RateHigh(bld.second, c_distanceGroups));
        } else {
            return false;
        }

        // Distance rating thresholds.
        static const std::vector<unsigned> c_undiscoveredGroups =
        { 2, 5, 10, 15, 20, 25, 30 };

        score.push_back(RateHigh(undiscovered, c_undiscoveredGroups));
    } break;

    case BLD_WOODCUTTER:
    {
        unsigned resources = world_.resources.GetReachable(pt, BResourceWood, true, true);

        // If it is not part of group it needs wood in range.
        if (building->GetGroup() == InvalidProductionGroup && 0 == resources)
            return false;

        unsigned distance = world_.GetMaxGroupMemberDistance(pt, building->GetGroup(), BLD_FORESTER);
        if (distance < std::numeric_limits<unsigned>::max() && distance > (FORESTER_RADIUS + 3))
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(RateHigh(resources, c_resourceGroups));
    } break;

    case BLD_QUARRY:
    {
        // Should be placed close to a storehouse with little stones.
        auto bld = world_.GetNearestBuilding(pt, c_storages);
        RTTR_Assert(bld.first.isValid()); // we always at least have a HQ.
        score.push_back(0.2*RateSmall(bld.second, c_distanceGroups)); // 0.2 because this is less important
    } break;

    case BLD_FORESTER:
    {
        // Must have a minimal distance to the group woodcutters
        unsigned distance = world_.GetMaxGroupMemberDistance(pt, building->GetGroup(), BLD_WOODCUTTER);
        if (distance < std::numeric_limits<unsigned>::max())
            if (distance > FORESTER_RADIUS + 3)
                return false;
    } break;

    case BLD_HUNTER:
    {
        // Should be far enough from other hunters.
        auto bld = world_.GetNearestBuilding(pt, { BLD_HUNTER });
        score.push_back(RateHigh(bld.second, { 4, 8, 12, 16, 20, 24 }));
    } break;

    case BLD_HARBORBUILDING:
    case BLD_STOREHOUSE:
    {
        // Should be far enough from other storages.
        auto bld = world_.GetNearestBuilding(pt, c_storages);
        score.push_back(RateHigh(bld.second, { 2, 3, 5, 7, 10, 12 }));
    } break;

    case BLD_SHIPYARD:
    {
        score.push_back(1.0);
    } break;

    default: break;
    }

    // Rate the amount of possible flag locations removed.
    unsigned possibleFlags = 0;
    for (unsigned dir = Direction::WEST; dir < Direction::EAST; ++dir) {
        MapPoint neighbour = world_.GetNeighbour(pt, Direction(dir));
        if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BQ_FLAG) {
            possibleFlags++;
        }
    }
    MapPoint flag = world_.GetNeighbour(pt, Direction::SOUTHEAST);
    for (unsigned dir = Direction::NORTHEAST; dir < Direction::COUNT; ++dir) {
        MapPoint neighbour = world_.GetNeighbour(flag, Direction(dir));
        if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BQ_FLAG) {
            possibleFlags++;
        }
    }
    MapPoint neighbour = world_.GetNeighbour(flag, Direction::WEST);
    if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BQ_FLAG) {
        possibleFlags++;
    }
    // There are less then 8 possible flag locations around.
    score.push_back(1. - (static_cast<double>(possibleFlags) / 8.));

    return true;
}

} // namespace beowulf
