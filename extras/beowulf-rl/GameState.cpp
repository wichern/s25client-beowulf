// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GameState.h"
#include "Environment.h"
#include "Settings.h"
#include "HeadlessGame.h"

#include "Game.h"
#include "gameData/BuildingProperties.h"
#include "gameData/JobConsts.h"
#include "gameData/GameConsts.h"
#include "GamePlayer.h"
#include "world/GameWorld.h"
#include "config.h"

namespace beowulf {

unsigned BuildingTypeWithoutUnused2I(BuildingType bld);
unsigned BuildingTypeWithoutUnused2I(BuildingType bld) {
    RTTR_Assert(BuildingProperties::IsValid(bld));
    unsigned ret = 0u;
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (bld == i)
            return ret;
        if (BuildingProperties::IsValid(i))
            ret++;
    }
    RTTR_Assert(false);
    return ret;
}

// GameState::GameState()
//     : data(arma::zeros<arma::colvec>(dimension))
// {

// }

GameState::GameState(Environment* env, const MapPoint& poi, BuildingQuality bq)
    : poi_(poi)
    , bq_(bq)
    , data(arma::zeros<arma::colvec>(dimension))
    , env_(env)
    , trees(data[7])
    , fish(data[8])
    , granite(data[9])
    , iron(data[10])
    , coal(data[11])
    , gold(data[12])
    , granite_underground(data[13])
    , player_territory(data[14])
    , enemy_territory(data[15])
    , visible_points(data[16])
    , bq_near(data[17])
    , bq_far(data[18])
    , distance_to_border(data[19])
    , distance_to_warehouse(data[20])
    , enemy_catapults(data[21])
    , water(data[22]) // see buildingTypeOffset below
    , buildingTypeOffset(23)
{

}

GameState& GameState::operator=(const GameState& other)
{
    if (this != &other) {
        isTerminal = other.isTerminal;
        poi_ = other.poi_;
        bq_ = other.bq_;
        data = other.data;
        env_ = other.env_;
    }
    return *this;
}

void GameState::Update()
{
    auto const& world = *env_->world_;
    auto const& player = world.GetPlayer(env_->agentId_);

    // some basic world parameters
    data[0] = world.GetWidth() * world.GetHeight();
    data[1] = world.GetNumPlayers();
    data[2] = poi_.x;
    data[3] = poi_.y;
    data[4] = player.GetHQPos().x;
    data[5] = player.GetHQPos().y;
    data[6] = static_cast<double>(bq_);

    if (!poi_.isValid()) {
        isTerminal = true;
        return;
    }
    
    const MapNode& node = world.GetNode(poi_);
    water = node.resources.getType() == ResourceType::Water ? node.resources.getAmount() : 0;
    
    distance_to_border = POI_RADIUS + 1; // distance to nearest border
    distance_to_warehouse = POI_RADIUS + 1; // distance to nearest warehouse

    world.CheckPointsInRadius(poi_, POI_RADIUS, [&](const MapPoint& pt, unsigned distance_to_poi) {
        const noBase* no = world.GetNO(pt);
        const MapNode& node = world.GetNode(pt);

        bool visible = world.CalcVisiblityWithAllies(pt, player.GetPlayerId()) == Visibility::Visible;
        if (visible)
            visible_points += 1.0;

        if (visible && distance_to_poi <= WOOD_WORK_RANGE)
            if (no && no->GetType() == NodalObjectType::Tree)
                trees += 1.0;
        if (visible && distance_to_poi <= STONE_WORK_RANGE)
            if (no && no->GetType() == NodalObjectType::Granite)
                granite += 1.0;
        if (visible && distance_to_poi <= FISHER_WORK_RANGE)
            if (node.resources.getType() == ResourceType::Fish)
                fish += node.resources.getAmount();
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

        if (world.IsPlayerTerritory(pt, player.GetPlayerId())) {
            player_territory += 1.0;

            // distance to border
            if (world.IsBorderNode(pt, player.GetPlayerId()))
                distance_to_border = std::min(distance_to_border, static_cast<double>(distance_to_poi));

            BuildingType buildingType = BuildingType::Nothing2;
            if (no && (no->GetType() == NodalObjectType::Building || no->GetType() == NodalObjectType::Buildingsite))
                buildingType = static_cast<const noBaseBuilding*>(no)->GetBuildingType();

            // distance to warehouse
            if (BuildingProperties::IsWareHouse(buildingType))
                distance_to_border = std::min(distance_to_border, static_cast<double>(distance_to_poi));

            BuildingQuality bq = world.GetBQ(pt, player.GetPlayerId());
            if (distance_to_poi < 3)
                bq_near += static_cast<unsigned>(bq);
            bq_far += static_cast<unsigned>(bq);

            // buildings nearby
            if (distance_to_poi < 6)
                if (BuildingProperties::IsValid(buildingType))
                    data[buildingTypeOffset + BuildingTypeWithoutUnused2I(buildingType)] += 1.0;
        } else {
            bool isEnemy = world.GetPlayer(node.owner - 1).IsAttackable(player.GetPlayerId());

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

    unsigned idx = buildingTypeOffset + helpers::MaxEnumValue_v<BuildingType> + 1 - NUM_UNUSED_BLD_TYPES;
    for(const auto i : helpers::enumRange<Job>())
        data[idx++] = player.GetInventory()[i];
    for(const auto i : helpers::enumRange<GoodType>())
        data[idx++] = player.GetInventory()[i];
    const auto buildingNums = player.GetBuildingRegister().GetBuildingNums();
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (BuildingProperties::IsValid(i)) {
            data[idx++] = buildingNums.buildings[i];
            data[idx++] = buildingNums.buildingSites[i];
            data[idx++] = player.GetBuildingRegister().CalcAverageProductivity(i);
        }
    }

    RTTR_Assert(idx == dimension);

    isTerminal = env_->engine_->em_.GetCurrentGF() > env_->settings_->maxGf || player.IsDefeated() || env_->engine_->game_.IsGameFinished();
}

} // namespace beowulf