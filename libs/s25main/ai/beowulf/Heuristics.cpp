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
static const BuildingLocationChecks S_build_location_checks[helpers::MaxEnumValue_v<BuildingType>] = {
   { 0, {},                     false }, // BuildingType::Headquarters
   { 0, {},                     false }, // BuildingType::Barracks
   { 0, {},                     false }, // BuildingType::Guardhouse
   { 0, {},                     false }, // BLD_NOTHING2
   { 0, {},                     false }, // BuildingType::Watchtower
   { 0, {},                     false }, // BLD_NOTHING3
   { 0, {},                     false }, // BLD_NOTHING4
   { 0, {},                     false }, // BLD_NOTHING5
   { 0, {},                     false }, // BLD_NOTHING6
   { 0, {},                     false }, // BuildingType::Fortress
   { 1, {},                     true  }, // BuildingType::GraniteMine
   { 1, {},                     true  }, // BuildingType::CoalMine
   { 1, {},                     true  }, // BuildingType::IronMine
   { 1, {},                     true  }, // BuildingType::GoldMine
   { 0, {},                     false }, // BuildingType::LookoutTower
   { 0, {},                     false }, // BLD_NOTHING7
   { 0, {},                     false }, // BuildingType::Catapult
   { 0, { BuildingType::Forester, BuildingType::Woodcutter, BuildingType::Sawmill }, true  }, // BuildingType::Woodcutter
   { 1, {},                     true  }, // BuildingType::Fishery
   { 1, {},                     true  }, // BuildingType::Quarry
   { 4, { BuildingType::Woodcutter },     true  }, // BuildingType::Forester
   { 0, { BuildingType::PigFarm },        true  }, // BuildingType::Slaughterhouse
   { 2, {},                     false }, // BuildingType::Hunter
   { 0, {},                     true  }, // BuildingType::Brewery
   { 0, { BuildingType::Ironsmelter },    true  }, // BuildingType::Armory
   { 0, {},                     true  }, // BuildingType::Metalworks
   { 0, {},                     true  }, // BuildingType::Ironsmelter
   { 8, {},                     true  }, // BuildingType::Charburner
   { 0, {},                     true  }, // BuildingType::PigFarm
   { 0, {},                     false }, // BuildingType::Storehouse
   { 0, {},                     false }, // BLD_NOTHING9
   { 0, {},                     true  }, // BuildingType::Mill
   { 0, { BuildingType::Mill },           true  }, // BuildingType::Bakery
   { 0, { BuildingType::Woodcutter },     true  }, // BuildingType::Sawmill
   { 0, {},                     true  }, // BuildingType::Mint
   { 1, {},                     true  }, // BuildingType::Well
   { 0, {},                     false }, // BuildingType::Shipyard
   {20, {},                     true  }, // BuildingType::Farm
   { 0, {},                     false }, // BuildingType::DonkeyBreeder
   { 0, {},                     false }, // BuildingType::HarborBuilding
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
    { BuildingType::Headquarters, BuildingType::Storehouse, BuildingType::HarborBuilding };

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
    if (world_.GetNearestBuilding(pt, { BuildingType::Farm, BuildingType::Charburner }, building).second < (2*FARMER_RADIUS))
        return false;
    else
        score.push_back(1.0);

    switch (building->GetType()) {
    case BuildingType::LookoutTower:
    {
        // Should have a lot of undiscovered area around.
        int undiscovered = 0;
        aii_.gwb.VisitPointsInRadius(pt, VISUALRANGE_LOOKOUTTOWER,
                                     [&](const MapPoint& pt)
        { if (!aii_.IsVisible(pt)) undiscovered++; }, false);

        if (undiscovered == 0)
            return false;

        // Should have enough distance to other towers.
        auto bld = world_.GetNearestBuilding(pt, { BuildingType::LookoutTower });
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

    case BuildingType::Woodcutter:
    {
        unsigned resources = world_.resources.GetReachable(pt, BResourceWood, true, true);

        // If it is not part of group it needs wood in range.
        if (building->GetGroup() == InvalidProductionGroup && 0 == resources)
            return false;

        unsigned distance = world_.GetMaxGroupMemberDistance(pt, building->GetGroup(), BuildingType::Forester);
        if (distance < std::numeric_limits<unsigned>::max() && distance > (FORESTER_RADIUS + 3))
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(RateHigh(resources, c_resourceGroups));
    } break;

    case BuildingType::Quarry:
    {
        // Should be placed close to a storehouse with little stones.
        auto bld = world_.GetNearestBuilding(pt, c_storages);
        RTTR_Assert(bld.first.isValid()); // we always at least have a HQ.
        score.push_back(0.2*RateSmall(bld.second, c_distanceGroups)); // 0.2 because this is less important
    } break;

    case BuildingType::Forester:
    {
        // Must have a minimal distance to the group woodcutters
        unsigned distance = world_.GetMaxGroupMemberDistance(pt, building->GetGroup(), BuildingType::Woodcutter);
        if (distance < std::numeric_limits<unsigned>::max())
            if (distance > FORESTER_RADIUS + 3)
                return false;
    } break;

    case BuildingType::Hunter:
    {
        // Should be far enough from other hunters.
        auto bld = world_.GetNearestBuilding(pt, { BuildingType::Hunter });
        score.push_back(RateHigh(bld.second, { 4, 8, 12, 16, 20, 24 }));
    } break;

    case BuildingType::HarborBuilding:
    case BuildingType::Storehouse:
    {
        // Should be far enough from other storages.
        auto bld = world_.GetNearestBuilding(pt, c_storages);
        score.push_back(RateHigh(bld.second, { 2, 3, 5, 7, 10, 12 }));
    } break;

    case BuildingType::Shipyard:
    {
        score.push_back(1.0);
    } break;

    default: break;
    }

    // Rate the amount of possible flag locations removed.
    unsigned possibleFlags = 0;
    for (unsigned dir = Direction::West; dir < Direction::East; ++dir) {
        MapPoint neighbour = world_.GetNeighbour(pt, Direction(dir));
        if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BuildingQuality::Flag) {
            possibleFlags++;
        }
    }
    MapPoint flag = world_.GetNeighbour(pt, Direction::SouthEast);
    for (unsigned dir = Direction::NorthEast; dir < Direction::COUNT; ++dir) {
        MapPoint neighbour = world_.GetNeighbour(flag, Direction(dir));
        if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BuildingQuality::Flag) {
            possibleFlags++;
        }
    }
    MapPoint neighbour = world_.GetNeighbour(flag, Direction::West);
    if (world_.IsOnRoad(neighbour) && world_.GetBQ(neighbour, false) == BuildingQuality::Flag) {
        possibleFlags++;
    }
    // There are less then 8 possible flag locations around.
    score.push_back(1. - (static_cast<double>(possibleFlags) / 8.));

    return true;
}

} // namespace beowulf
