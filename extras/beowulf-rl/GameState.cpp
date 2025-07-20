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
#include "ai/beowulf/Helper.h"
#include "GamePlayer.h"
#include "world/GameWorld.h"
#include "config.h"

namespace beowulf {

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
    data[0] = std::min(1.0, static_cast<double>((world.GetWidth() * world.GetHeight()) / (512*512)));
    data[1] = std::min(1.0, static_cast<double>(world.GetNumPlayers()) / 8.0);
    data[2] = static_cast<double>(poi_.x / world.GetWidth());
    data[3] = static_cast<double>(poi_.y / world.GetHeight());
    data[4] = static_cast<double>(player.GetHQPos().x / world.GetWidth());
    data[5] = static_cast<double>(player.GetHQPos().y / world.GetHeight());
    data[6] = static_cast<double>(bq_) / static_cast<double>(helpers::MaxEnumValue_v<BuildingQuality>);

    if (!poi_.isValid()) {
        isTerminal = true;
        return;
    }

    const MapNode& node = world.GetNode(poi_);
    water = node.resources.getType() == ResourceType::Water ? 1.0 : 0.0;

    distance_to_border = POI_RADIUS + 1; // distance to nearest border
    distance_to_warehouse = POI_RADIUS + 1; // distance to nearest warehouse

    PathConditionHuman walkable(world);
    static constexpr unsigned MaxWalkDistance = std::max({WOOD_WORK_RANGE, STONE_WORK_RANGE, FISHER_WORK_RANGE});

    // Check all walkable destinations for trees, granite, fish and animals
    FloodFill(world, poi_,
    // condition (walkable)
    [&](const MapPoint& pt, Direction dir)
    { 
        if (world.CalcDistance(poi_, pt) > MaxWalkDistance)
            return false;
        return walkable.IsNodeOk(pt) && walkable.IsEdgeOk(pt, dir);
    },
    // action
    [&](const MapPoint& pt)
    {
        const noBase* no = world.GetNO(pt);
        const MapNode& node = world.GetNode(pt);

        const unsigned distance = world.CalcDistance(poi_, pt);

        if (no && world.CalcVisiblityWithAllies(pt, player.GetPlayerId()) == Visibility::Visible) {
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

    world.CheckPointsInRadius(poi_, POI_RADIUS, [&](const MapPoint& pt, unsigned distance_to_poi) {
        const noBase* no = world.GetNO(pt);
        const MapNode& node = world.GetNode(pt);

        bool visible = world.CalcVisiblityWithAllies(pt, player.GetPlayerId()) == Visibility::Visible;
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
                distance_to_warehouse = std::min(distance_to_warehouse, static_cast<double>(distance_to_poi));

            BuildingQuality bq = world.GetBQ(pt, player.GetPlayerId());
            if (distance_to_poi < 3)
                bq_near += static_cast<double>(bq);
            bq_far += static_cast<double>(bq);

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
    
#if 0
    const MapNode& node = world.GetNode(poi_);
    water = node.resources.getType() == ResourceType::Water ? node.resources.getAmount() : 0;
    
    distance_to_border = POI_RADIUS + 1; // distance to nearest border
    distance_to_warehouse = POI_RADIUS + 1; // distance to nearest warehouse

    // @todo: only consider points we can walk to (BFS) at least for most resources (how is fish handled?)
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
#endif

    unsigned idx = buildingTypeOffset + helpers::MaxEnumValue_v<BuildingType> + 1 - NUM_UNUSED_BLD_TYPES;
    for(const auto i : helpers::enumRange<Job>())
        data[idx++] = std::min(1.0, static_cast<double>(player.GetInventory()[i]) / 100.0);
    for(const auto i : helpers::enumRange<GoodType>())
        data[idx++] = std::min(1.0, static_cast<double>(player.GetInventory()[i]) / 100.0);
    const auto buildingNums = player.GetBuildingRegister().GetBuildingNums();
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (BuildingProperties::IsValid(i)) {
            data[idx++] = std::min(1.0, static_cast<double>(buildingNums.buildings[i]) / 20.0);
            data[idx++] = std::min(1.0, static_cast<double>(buildingNums.buildingSites[i]) / 20.0);
            data[idx++] = std::min(1.0, static_cast<double>(player.GetBuildingRegister().CalcAverageProductivity(i)) / 100.0);
        }
    }

    RTTR_Assert(idx == dimension);

    for (unsigned i = 0; i < data.size(); ++i) 
    {
        RTTR_Assert(data[i] >= 0.0 && data[i] <= 1.0);
    }

    isTerminal = env_->engine_->game.IsGameFinished();
}

} // namespace beowulf