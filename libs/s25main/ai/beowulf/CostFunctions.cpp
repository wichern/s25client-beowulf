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

#include "ai/beowulf/CostFunctions.h"
#include "ai/beowulf/BuildingQualityCalculator.h"
#include "ai/beowulf/ResourceMap.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Helper.h"

#include "ai/AIInterface.h"
#include "world/GameWorldBase.h"
#include "buildings/nobBaseWarehouse.h"
#include "gameData/BuildingConsts.h"
#include "gameTypes/Inventory.h"

namespace beowulf {

BuildingPositionCosts::BuildingPositionCosts(const AIInterface& aii,
                                             BuildingQualityCalculator& bqc,
                                             ResourceMap& resourceMap,
                                             const BuildingsBase* buildings)
    : aii_(aii),
      bqc_(bqc),
      resourceMap_(resourceMap),
      buildings_(buildings)
{

}

double Rate(unsigned val, const std::vector<unsigned>& groups)
{
    unsigned count = 0;
    while (val > groups[count] && count < groups.size()) count++;
    return 1.0 - (1.0 / (static_cast<double>(groups.size() + 2) - static_cast<double>(count)));
}

bool BuildingPositionCosts::Score(
        std::vector<double>& score,
        const Building* building,
        const MapPoint& pt,
        Island island)
{
    // Set of known storage building types.
    static const std::vector<BuildingType> c_storages =
    { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING };

    // Distance rating thresholds.
    static const std::vector<unsigned> c_distanceGroups =
    { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };

    // Resouces available rating thresholds.
    static const std::vector<unsigned> c_resourceGroups =
    { 1, 2, 3, 5, 8, 13 };

    const MapBase& world = resourceMap_;

    switch (building->GetType()) {
    case BLD_GRANITEMINE:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceGranite) == 0)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceGranite), c_resourceGroups));

        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_COALMINE:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceCoal) == 0)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceCoal), c_resourceGroups));

        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_IRONMINE:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceIron) == 0)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceIron), c_resourceGroups));

        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_GOLDMINE:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceGold) == 0)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceGold), c_resourceGroups));

        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

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
        auto bld = buildings_->GetNearestBuilding(world, pt, { BLD_LOOKOUTTOWER }, island);
        if (bld.first.isValid() && bld.second < (VISUALRANGE_LOOKOUTTOWER/3)) {
            score.push_back(1.0 - Rate(bld.second, c_distanceGroups));
        } else {
            return false;
        }

        // Distance rating thresholds.
        static const std::vector<unsigned> c_undiscoveredGroups =
        { 2, 5, 10, 15, 20, 25, 30 };

        score.push_back(Rate(undiscovered, c_undiscoveredGroups));
    } break;

    case BLD_WOODCUTTER:
    {
        // If it is not part of group it needs wood in range.
        if (building->GetGroup() == InvalidProductionGroup)
            if (resourceMap_.Get(pt, BResourceWood) == 0)
                return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceWood), c_resourceGroups));

        // Should be placed close to group forester.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_FORESTER });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_FISHERY:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceFish) == 0)
            return false;
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceFish), c_resourceGroups));

        // Should have enough distance to other fishery.
        auto bld = buildings_->GetNearestBuilding(world, pt, { BLD_FISHERY }, island);
        if (bld.first.isValid() && bld.second < 8) // @todo: 8 is a wild guess.
            score.push_back(1.0 - Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_QUARRY:
    {
        // Only viable if enough resources are available.
        if (resourceMap_.Get(pt, BResourceStone) == 0)
            return false;

        // Should be placed where a lot of resources are available.
        score.push_back(Rate(resourceMap_.Get(pt, BResourceStone), c_resourceGroups));

        // Should be placed close to a storehouse with little stones.
        auto bld = buildings_->GetNearestBuilding(world, pt, c_storages, island);
        RTTR_Assert(bld.first.isValid()); // we always at least have a HQ.
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should have enough distance to other quarry.
        bld = buildings_->GetNearestBuilding(world, pt, { BLD_QUARRY }, island);
        if (bld.first.isValid() && bld.second < 8) // @todo: 8 is a wild guess.
            score.push_back(1.0 - Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_FORESTER:
    {
        // Check that minimum amount of plantspace is available.
        if (resourceMap_.Get(pt, BResourcePlantSpace_6) <= 6)
            return false;

        // Should be placed close to group woodcutter.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WOODCUTTER });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed where there is a lot of plantspace.
        score.push_back(resourceMap_.Get(pt, BResourcePlantSpace_6) > 10 ? 1.0 : 0.5);

        // Should be far enough from farms.
        bld = buildings_->GetNearestBuilding(world, pt, { BLD_FARM }, island);
        score.push_back(1.0 - Rate(bld.second, { 2, 3, 5, 7, 10, 12 })); // @todo: 10 is a wild guess.
    } break;

    case BLD_SLAUGHTERHOUSE:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group pigfarm.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_PIGFARM });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_HUNTER:
    {
        // Check that minimum amount of huntable animals are available.
        if (resourceMap_.Get(pt, BResourceHuntableAnimals) <= 6)
            return false;

        // Should be placed where there is a lot of huntable animals .
        score.push_back(resourceMap_.Get(pt, BResourceHuntableAnimals) > 4 ? 1.0 : 0.5);

        // Should be far enough from hunters.
        auto bld = buildings_->GetNearestBuilding(world, pt, { BLD_HUNTER }, island);
        score.push_back(1.0 - Rate(bld.second, { 4, 8, 12, 16, 20, 24 }));
    } break;

    case BLD_BREWERY:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farms.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_FARM });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to group wells.
        bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WELL });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_ARMORY:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farms.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_IRONSMELTER });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_METALWORKS:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farms.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_IRONSMELTER });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_IRONSMELTER:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_CHARBURNER:
    {
        // Check that minimum amount of plantspace is available.
        if (resourceMap_.Get(pt, BResourcePlantSpace_2) <= 2) // @todo: check what AIJH does
            return false;

        // Should be placed where there is a lot of plantspace.
        score.push_back(resourceMap_.Get(pt, BResourcePlantSpace_2) > 4 ? 1.0 : 0.5);

        // Should be far enough from farms.
        auto bld = buildings_->GetNearestBuilding(world, pt, { BLD_FARM }, island);
        score.push_back(1.0 - Rate(bld.second, { 2, 3, 5, 7 })); // @todo: 7 is a wild guess.
    } break;

    case BLD_PIGFARM:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farms.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_FARM });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to group wells.
        bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WELL });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_HARBORBUILDING:
    case BLD_STOREHOUSE:
    {
        // Should be far enough from other storages.
        auto bld = buildings_->GetNearestBuilding(world, pt, c_storages, island);
        score.push_back(1.0 - Rate(bld.second, { 2, 3, 5, 7, 10, 12 }));

        // @todo: should be far from front
    } break;

    case BLD_MILL:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farms.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_FARM });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to group wells.
        bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WELL });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_BAKERY:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group mill.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_MILL });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to group wells.
        bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WELL });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_SAWMILL:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group woodcutter.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WOODCUTTER });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    case BLD_MINT:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // @todo: should be placed far from enemy land
    } break;

    case BLD_WELL:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
    } break;

    case BLD_SHIPYARD:
    {
        score.push_back(1.0);
    } break;

    case BLD_FARM:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);
    } break;

    case BLD_DONKEYBREEDER:
    {
        // Should be placed close to destination of its wares.
        Building* dst = buildings_->GetGoodsDest(world, building, island, pt);
        if (dst)
            score.push_back(Rate(dst->GetDistance(pt), c_distanceGroups));
        else
            score.push_back(0.1);

        // Should be placed close to group farm.
        auto bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_FARM });
        score.push_back(Rate(bld.second, c_distanceGroups));

        // Should be placed close to group wells.
        bld = buildings_->GroupMemberDistance(pt, building->GetGroup(), { BLD_WELL });
        score.push_back(Rate(bld.second, c_distanceGroups));
    } break;

    default:
    {
        RTTR_Assert(false);
        return false;
    } break;
    }

    return true;
}

} // namespace beowulf
