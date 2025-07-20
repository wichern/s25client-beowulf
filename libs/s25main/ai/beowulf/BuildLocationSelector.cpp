// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildLocationSelector.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/BeowulfConsts.h"

#include "ai/AIInterface.h"
#include "world/GameWorldBase.h"
#include "buildings/nobBaseWarehouse.h"
#include "gameData/BuildingConsts.h"
#include "gameData/BuildingProperties.h"
#include "gameData/GameConsts.h"
#include "gameData/JobConsts.h"
#include "pathfinding/PathConditionHuman.h"

namespace beowulf {

BuildLocationSelector::BuildLocationSelector()
{
    // for now, create the same network for every building type
    for (ffn_t& network : networks_) {
        network.Add(new mlpack::Linear(state_dim));
        network.Add(new mlpack::ReLU());
        // network_.Add(new mlpack::Linear(8));
        // network_.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(1));
    }
}

BuildLocationSelector::~BuildLocationSelector()
{
    
}

MapPoint BuildLocationSelector::Select(
    AIInterface& aii,
    const nobBaseWarehouse* warehouse,
    BuildingType bld,
    bool random)
{
    RTTR_Assert(warehouse);

    BuildLocations locations(aii);
    locations.Calculate(warehouse->GetFlagPos());
    
    const BuildingQuality bq = BUILDING_SIZE[bld];

    // We stack all vectors into a matrix for batch prediction
    const auto buildLocationVec = locations.Get(bq);

    if (buildLocationVec.empty())
        return MapPoint();

    arma::mat states(state_dim, buildLocationVec.size());
    for (size_t i = 0; i < buildLocationVec.size(); ++i)
        states.col(i) = CreateState(aii.gwb, aii.GetPlayerId(), buildLocationVec[i], bld, bq);
    
    // Predict scores
    arma::mat prediction;
    networks_[BuildingTypeWithoutUnused2I(bld)].Predict(states, prediction);

    // Choose best location
    size_t chosenIdx;
    if (random)
        chosenIdx = mlpack::RandInt(buildLocationVec.size());
    else
        chosenIdx = prediction.index_max();  // get best result

    // @todo: Specify a minimum threshold below which we do not place buildings, because they do more harm than good.
    //        Better wait for the game to progress in that case.

    return buildLocationVec[chosenIdx];
}

arma::colvec BuildLocationSelector::CreateState(
    const GameWorldBase& gwb,
    const unsigned playerId,
    const MapPoint& pt,
    BuildingType bld,
    BuildingQuality bq)
{
    arma::colvec ret(arma::zeros<arma::colvec>(state_dim));

    PathConditionHuman walkable(gwb);
    static constexpr unsigned MaxWalkDistance = std::max({WOOD_WORK_RANGE, STONE_WORK_RANGE, FISHER_WORK_RANGE});

    unsigned idx = 0u;

    ret[idx++] = static_cast<double>(bld) / static_cast<double>(helpers::MaxEnumValue_v<BuildingType>);
    ret[idx++] = static_cast<double>(bq) / static_cast<double>(helpers::MaxEnumValue_v<BuildingQuality>);

    double& water = ret[idx++];
    water = gwb.GetNode(pt).resources.getType() == ResourceType::Water ? 1.0 : 0.0;
    double& trees = ret[idx++];
    double& granite = ret[idx++];
    double& fish = ret[idx++];

    // Check all walkable destinations for trees, granite, fish and animals
    FloodFill(gwb, pt,
    // condition (walkable)
    [&](const MapPoint& pt2, Direction dir)
    { 
        if (gwb.CalcDistance(pt, pt2) > MaxWalkDistance)
            return false;
        return walkable.IsNodeOk(pt2) && walkable.IsEdgeOk(pt2, dir);
    },
    // action
    [&](const MapPoint& pt2)
    {
        const noBase* no = gwb.GetNO(pt2);
        const MapNode& node = gwb.GetNode(pt2);

        const unsigned distance = gwb.CalcDistance(pt, pt2);

        if (no && gwb.CalcVisiblityWithAllies(pt2, playerId) == Visibility::Visible) {
            switch (no->GetType())
            {
            case NodalObjectType::Tree:
            {
                if (distance <= WOOD_WORK_RANGE)
                    trees += 1.0;
            } break;
            case NodalObjectType::Granite:
            {
                if (distance <= STONE_WORK_RANGE)
                    granite += 1.0;
            } break;
            default: break;
            }

            if (node.resources.getType() == ResourceType::Fish)
                if (distance <= FISHER_WORK_RANGE)
                    fish += node.resources.getAmount();
            
            // @todo: animals
        }
    }
    );

    double& visible_points = ret[idx++];
    double& iron = ret[idx++];
    double& gold = ret[idx++];
    double& coal = ret[idx++];
    double& granite_underground = ret[idx++];
    double& player_territory = ret[idx++];
    double& distance_to_border = ret[idx++];
    double& distance_to_warehouse = ret[idx++];
    double& bq_near = ret[idx++];
    double& bq_far = ret[idx++];
    double& enemy_territory = ret[idx++];
    double& enemy_catapults = ret[idx++];

    gwb.CheckPointsInRadius(pt, POI_RADIUS, [&](const MapPoint& pt, unsigned distance_to_poi) {
        const noBase* no = gwb.GetNO(pt);
        const MapNode& node = gwb.GetNode(pt);

        bool visible = gwb.CalcVisiblityWithAllies(pt, playerId) == Visibility::Visible;
        if (visible)
            visible_points += 1.0;

        if (visible && distance_to_poi <= MINER_RADIUS) {
            if (node.resources.getType() == ResourceType::Iron)
                iron += node.resources.getAmount();
            if (node.resources.getType() == ResourceType::Gold)
                gold += node.resources.getAmount();
            if (node.resources.getType() == ResourceType::Coal)
                coal += node.resources.getAmount();
            if (node.resources.getType() == ResourceType::Granite)
                granite_underground += node.resources.getAmount();
        }

        if (gwb.IsPlayerTerritory(pt, playerId)) {
            player_territory += 1.0;

            // distance to border
            if (!gwb.IsPlayerTerritory(pt, playerId + 1))
                distance_to_border = std::min(distance_to_border, static_cast<double>(distance_to_poi));

            BuildingType buildingType = BuildingType::Nothing2;
            if (no && (no->GetType() == NodalObjectType::Building || no->GetType() == NodalObjectType::Buildingsite))
                buildingType = static_cast<const noBaseBuilding*>(no)->GetBuildingType();

            // distance to warehouse
            if (BuildingProperties::IsWareHouse(buildingType))
                distance_to_warehouse = std::min(distance_to_warehouse, static_cast<double>(distance_to_poi));

            BuildingQuality bq = gwb.GetBQ(pt, playerId);
            if (distance_to_poi < 3)
                bq_near += static_cast<double>(bq);
            bq_far += static_cast<double>(bq);

            // buildings nearby
            if (distance_to_poi < 6)
                if (BuildingProperties::IsValid(buildingType))
                    ret[idx + BuildingTypeWithoutUnused2I(buildingType)] += 1.0;
        } else {
            bool isEnemy = gwb.GetPlayer(node.owner - 1).IsAttackable(playerId);

            if (isEnemy)
                enemy_territory += 1.0;

            // Enemy catapult in range
            if (distance_to_poi < CATAPULT_RANGE)
                if (no && (no->GetType() == NodalObjectType::Building || no->GetType() == NodalObjectType::Buildingsite))
                    if (static_cast<const noBaseBuilding*>(no)->GetBuildingType() == BuildingType::Catapult)
                        enemy_catapults += 1.0;
        }

        return true;
    }, true);

    // Normalize
    trees = std::min(1.0, trees / 40.0); // what are the maximum amount of trees?
    fish = std::min(1.0, fish / 40.0);
    granite = std::min(1.0, granite / 40.0);
    iron = std::min(1.0, iron / 10.0);
    coal = std::min(1.0, coal / 10.0);
    gold = std::min(1.0, gold / 10.0);
    granite_underground = std::min(1.0, granite_underground / 10.0);

    static constexpr double PointsInRange = static_cast<double>(POI_RADIUS * POI_RADIUS + POI_RADIUS) * 3.0;
    player_territory = std::min(1.0, player_territory / PointsInRange);
    enemy_territory = std::min(1.0, enemy_territory / PointsInRange);
    visible_points = std::min(1.0, visible_points / PointsInRange);

    bq_near = std::min(1.0, bq_near / 50.0);
    bq_far = std::min(1.0, bq_near / 500.0);

    distance_to_border /= static_cast<double>(POI_RADIUS + 1);
    distance_to_warehouse /= static_cast<double>(POI_RADIUS + 1);
    
    enemy_catapults = std::min(1.0, enemy_catapults / 3.0);

    RTTR_Assert(idx == state_dim);

    // @todo: add stats of buildings around

    return ret;
}

} // namespace beowulf