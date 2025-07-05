// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Environment.h"
#include "HeadlessGame.h"
#include "gameData/BuildingProperties.h"
#include "gameData/BuildingConsts.h"
#include "gameTypes/GameTypesOutput.h"
#include "PointOutput.h"
#include "EventManager.h"
#include "Settings.h"
#include "Observer.h"
#include "AsciiMap.h"
#include "ai/AIInterface.h"
#include "ai/beowulf/BuildLocations.h"
#include "FindWhConditions.h"
#include "buildings/nobHQ.h"

namespace beowulf {

BuildingType BuildingTypeWithoutUnused(unsigned bld);
BuildingType BuildingTypeWithoutUnused(unsigned bld) {
    for(const auto i : helpers::enumRange<BuildingType>()) {
        if (!BuildingProperties::IsValid(i))
            continue;
        if (0 == bld)
            return i;
        bld--;
    }
    RTTR_Assert(false);
    return BuildingType::Nothing2;
}

Environment::Environment(Settings* settings)
: settings_(settings)
{
    RTTR_Assert(settings_);
}

Environment::State Environment::InitialSample()
{
    engine_ = std::make_unique<HeadlessGame>(settings_);
    world_ = &engine_->world_;
    
    for (unsigned i = 0u; i < world_->GetNumPlayers(); ++i) {
        if (world_->GetPlayer(i).aiInfo.type == AI::Type::Beowulf) {
            agentId_ = i;
            break;
        }
    }

    Environment::State ret = Environment::State(this, GetNextPOI());
    ret.Update();
    return ret;
}

bool Environment::IsTerminal(const State& /*state*/) const
{
    // @todo: if this is being called by replay, we need to store the information in the state instead.
    return engine_->em_.GetCurrentGF() > settings_->maxGf || world_->GetPlayer(agentId_).IsDefeated() || engine_->game_.IsGameFinished();
}

size_t Environment::ActionSize() const { return Action::size; }
size_t Environment::StateSize() const { return State::dimension; }

double Environment::Sample(const State& state,
    const Action& action,
    State& nextState)
{
    double ret = 0.0;

    if (action.action != 0)
    {
        BuildingType bld = BuildingTypeWithoutUnused(action.action - 1);
        auto& aii = engine_->players_[agentId_]->getAIInterface();

        if(!canUseBq(world_->GetBQ(state.poi_, agentId_), BUILDING_SIZE[bld]))
            ret -= 0.5;
        else
            aii.SetBuildingSite(state.poi_, bld);
    }

    MetaState oldMetaState = ExtractMetaState();
    
    // advance state
    MapPoint nextPoi;
    do {
        engine_->RunNextNWGF();
        nextPoi = GetNextPOI();
        beowulf::Observer::getInstance().setCurrentGf(engine_->em_.GetCurrentGF());
        beowulf::Observer::getInstance().printState();
    } while (!IsTerminal(state) && !nextPoi.isValid());
    

    nextState = GameState(this, nextPoi);
    nextState.Update();

    MetaState nextMetaState = ExtractMetaState();

    // Evaluate reward
    ret += RewardGameState(oldMetaState, nextMetaState);
    ret += RewardNewGoods(oldMetaState, nextMetaState);
    ret += RewardNewConnections(oldMetaState, nextMetaState);
    ret += RewardNewBuildings(oldMetaState, nextMetaState);

    return ret;
}

Environment::MetaState Environment::ExtractMetaState() const
{
    auto const& player = world_->GetPlayer(agentId_);
    auto& aii = engine_->players_[agentId_]->getAIInterface();

    MetaState ret;

    // IMPORTANT: everything checked here should most likely be in the state data array as well

    ret.goods = player.GetInventory().goods;
    ret.people = player.GetInventory().people;
    ret.constructionSiteCount = player.GetBuildingRegister().GetBuildingSites().size();

    ret.buildingCount = 0u;
    ret.connectedBuildings = 0u;
    for(const auto buildingType : helpers::enumRange<BuildingType>())
    {
        if (BuildingProperties::IsUsual(buildingType))
        {
            auto const& buildings = player.GetBuildingRegister().GetBuildings(buildingType);
            ret.buildingCount = buildings.size();

            if (buildingType == BuildingType::Headquarters)
                continue;
            
            if (!player.GetHQPos().isValid())
                continue;
            
            const auto* hq = aii.GetHeadquarter();
            if (!hq)
                continue;
            
            // @todo: if this calculation is slow, we can buffer it?
            for (const auto* building : buildings)
            {
                if (BuildingProperties::IsWareHouse(building->GetBuildingType()))
                {
                    if (aii.FindPathOnRoads(*building->GetFlag(), *hq, nullptr))
                        ret.connectedBuildings++;
                }
                else
                {
                    if (aii.FindWarehouse(*building->GetFlag(), FW::NoCondition(), true, true))
                        ret.connectedBuildings++;   
                }
            }
        }
    }
    
    ret.defeated = player.IsDefeated();

    return ret;
}

double Environment::RewardGameState(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;
    if (!oldState.defeated && newState.defeated)
        ret -= 100.0;
    return ret;
}

inline double delta(unsigned oldVal, unsigned newVal)
{
    if (oldVal >= newVal)
        return 0.0;
    return static_cast<double>(newVal - oldVal);
}

double Environment::RewardNewGoods(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;
    for(const auto i : helpers::enumRange<GoodType>())
    {
        double reward = GOOD_GAIN_REWARD[i];
        if (reward > 0.0)
            ret += delta(oldState.goods[i], newState.goods[i]) * reward;
    }
    return ret;
}

double Environment::RewardNewPeople(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;
    (void)oldState;
    (void)newState;
    // @todo
    return ret;
}

double Environment::RewardNewConnections(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;

    // reward new connections
    ret += delta(oldState.connectedBuildings, newState.connectedBuildings) * 0.005;

    // punish lost connections
    ret -= delta(newState.connectedBuildings, oldState.connectedBuildings) * 0.0025;

    return ret;
}

double Environment::RewardNewBuildings(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;
    ret += delta(oldState.constructionSiteCount, newState.constructionSiteCount) * 0.01;
    ret += delta(oldState.buildingCount, newState.buildingCount) * 0.02;
    return ret;
}

MapPoint Environment::GetNextPOI()
{
    while (!poi.buildLocations.empty()) {
        MapPoint ret = poi.buildLocations.back();
        poi.buildLocations.pop_back();

        // still valid?
        BuildingQuality bq = world_->GetBQ(ret, agentId_);
        if (bq > BuildingQuality::Nothing)
            return ret;
    }

    // calculate next POIS
    auto& aii = engine_->players_[agentId_]->getAIInterface();
    BuildLocations buildLocations(aii);
    for (const auto* sh : aii.GetStorehouses())
        buildLocations.Calculate(sh->GetFlagPos());
    poi.buildLocations = std::move(buildLocations.Get());

    if (!poi.buildLocations.empty()) {
        MapPoint ret = poi.buildLocations.back();
        poi.buildLocations.pop_back();
        return ret;
    }

    // There are no available build locations anymore
    return MapPoint();
}

} // namespace beowulf
