// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GameState.h"
#include "Environment.h"
#include "Settings.h"
#include "HeadlessGame.h"

#include "Game.h"
#include "GamePlayer.h"
#include "world/GameWorld.h"
#include "config.h"

namespace beowulf {

GameState::GameState()
    : data(arma::zeros<arma::colvec>(dimension))
{

}

GameState::GameState(Environment* env)
    : data(arma::zeros<arma::colvec>(dimension))
    , env_(env)
{
    poi_ = env_->world_->GetPlayer(env_->agentId_).GetHQPos();
    Update();
}

void GameState::Update()
{
    auto const& world = *env_->world_;
    auto const& player = world.GetPlayer(env_->agentId_);
    
    // Insert meta informations
    unsigned idx = 0u;

    // Start with currently selected action, the three parameters
    data[idx++] = actionStep_;
    for (unsigned p : actionParams_)
        data[idx++] = p;

    // some basic world parameters
    data[idx++] = world.GetWidth() * world.GetHeight();
    data[idx++] = world.GetNumPlayers();
    data[idx++] = poi_.x;
    data[idx++] = poi_.y;
    for(const auto i : helpers::enumRange<Job>())
        data[idx++] = player.GetInventory()[i];
    for(const auto i : helpers::enumRange<GoodType>())
        data[idx++] = player.GetInventory()[i];
    RTTR_Assert(idx == 1 + MAX_PARAMS_PER_ACTION + meta_information_count);

    // Insert map point data
    if (poi_.isValid()) {
        auto const points = world.GetPointsInRadiusWithCenter(poi_, POI_RADIUS);
        for (size_t i = 0; i < points.size(); ++i) {
            auto const& node = world.GetNode(points[i]);

            unsigned offset = meta_information_count + (i *  point_attribute_count);
            
            idx = offset;
            data[idx++] = static_cast<double>(node.fow[env_->agentId_].visibility);

            if (node.fow[env_->agentId_].visibility == Visibility::Invisible)
                continue;

            // @todo: encode roads into one int in order to compress?
            data[idx++] = static_cast<double>(node.roads[RoadDir::East]);
            data[idx++] = static_cast<double>(node.roads[RoadDir::SouthEast]);
            data[idx++] = static_cast<double>(node.roads[RoadDir::SouthWest]);
            data[idx++] = (node.altitude); // @todo: is this even neccessary?
            // @todo: add terrain information required for farms
            // @todo: do we know the resources?
            // @todo: compress type and amount into one integer?
            data[idx++] = static_cast<double>(node.resources.getType());
            data[idx++] = static_cast<double>(node.resources.getAmount());
            data[idx++] = (node.owner);

            auto const* no = world.GetNO(points[i]);
            data[idx++] = static_cast<double>(no->GetType());
            if (no->GetType() == NodalObjectType::Building || no->GetType() == NodalObjectType::Buildingsite) {
                auto const* noBuilding = static_cast<noBaseBuilding const*>(no);
                data[idx++] = static_cast<double>(noBuilding->GetBuildingType());
            } else {
                idx++;
            }

            RTTR_Assert(idx == offset + point_attribute_count);
            
            // @todo: do we need to know about figures? it is not a fixed amount!
            // @todo: Add wares on roads? Or road usage?
            // @todo: V2: add distance to nearest known enemy catapult closer than 14 points distance (catapult range)
        }
    }

    isTerminal = env_->engine_->em_.GetCurrentGF() > env_->settings_->maxGf || player.IsDefeated() || env_->engine_->game_.IsGameFinished();
}

} // namespace beowulf