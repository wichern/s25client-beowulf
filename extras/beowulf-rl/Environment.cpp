// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Environment.h"
#include "HeadlessGame.h"
#include "gameData/BuildingProperties.h"
#include "gameData/BuildingConsts.h"
#include "gameTypes/GameTypesOutput.h"
#include "addons/const_addons.h"
#include "PointOutput.h"
#include "EventManager.h"
#include "Settings.h"
#include "Observer.h"
#include "AsciiMap.h"
#include "ai/AIInterface.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/RoadBuilder.h"
#include "ai/beowulf/Helper.h"
#include "FindWhConditions.h"
#include "buildings/nobHQ.h"

namespace beowulf {

Environment::Environment(Settings* settings, beowulf::Observer* observer)
: settings_(settings)
, observer_(observer)
{

}

Environment::State Environment::InitialSample()
{
    engine_ = std::make_unique<HeadlessGame>(*settings_);
    world_ = &(engine_->world);
    
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
    return engine_->IsFinished();
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
        const BuildingType bld = BuildingTypeWithoutUnused(action.action - 1);
        auto& player = world_->GetPlayer(agentId_);
        bool isAllowed = true;

        // check if this building is allowed according to settings an mods
        switch (bld)
        {
        case BuildingType::Catapult:
        {
            isAllowed = player.CanBuildCatapult();
        } break;
        case BuildingType::Charburner:
        {
            isAllowed = settings_->ggs.isEnabled(AddonId::CHARBURNER);
        } break;
        case BuildingType::Winery:
        case BuildingType::Temple:
        case BuildingType::Vineyard:
        {
            isAllowed = settings_->ggs.isEnabled(AddonId::WINE);
        } break;
        default: break;
        }

        if (isAllowed) {
            auto& aii = engine_->AII();
            const BuildingQuality bq = BUILDING_SIZE[bld];

            // punish too many building sites
            ret -= static_cast<double>(player.GetBuildingRegister().GetBuildingSites().size()) * 0.00001;

            if(canUseBq(world_->GetBQ(state.poi_, agentId_), bq)) {
                // Can we build the road?
                RoadBuilder roads(aii, state.poi_, bq);
                MapPoint flagPos = world_->GetNeighbour(state.poi_, Direction::SouthEast);

                if (roads.IsConnected(flagPos, true)) {
                    aii.SetBuildingSite(state.poi_, bld);
                    observer_->OnSetBuildingSite(state.poi_, bld);
                } else {
                    std::vector<Direction> route;
                    if (roads.FindConnectionToNearestFlag(flagPos, &route)) {
                        aii.SetBuildingSite(state.poi_, bld);
                        aii.BuildRoad(flagPos, false, route);
                        observer_->OnSetBuildingSite(state.poi_, bld);
                    }
                }
            }
        } else {
            ret -= 0.02;
        }
    }

    MetaState oldMetaState = ExtractMetaState();
    
    // advance state until we want to perform the next action.
    MapPoint nextPoi;
    do {
        engine_->RunNextNWGF();
        nextPoi = GetNextPOI();
        observer_->SetCurrentGf(engine_->em.GetCurrentGF());
        observer_->Print(engine_->world, engine_->AgentPlayer().GetPlayerId());
    } while (!IsTerminal(state) && !nextPoi.isValid());
    

    nextState = GameState(this, nextPoi);
    nextState.Update();

    MetaState nextMetaState = ExtractMetaState();

    // Evaluate reward
    ret += RewardGameState(oldMetaState, nextMetaState);
    ret += RewardNewGoods(oldMetaState, nextMetaState);
    ret += RewardNewBuildings(oldMetaState, nextMetaState);

    // Ideas: provide some additional wisdom:
    //  - woodcutter count should be close to 2*sawmill count
    //  - stonemasen count should be close to sawmill count

    return ret;
}

Environment::MetaState Environment::ExtractMetaState() const
{
    auto const& player = world_->GetPlayer(agentId_);
    auto& aii = engine_->AII();

    MetaState ret;

    // IMPORTANT: everything checked here should most likely be in the state data array as well

    ret.goods = player.GetInventory().goods;
    ret.people = player.GetInventory().people;
    ret.constructionSiteCount = player.GetBuildingRegister().GetBuildingSites().size();

    ret.buildingCount = 0u;
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

inline double delta(const unsigned oldVal, const unsigned newVal)
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
        const double reward = GOOD_GAIN_REWARD[i];
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

double Environment::RewardNewBuildings(const MetaState& oldState, const MetaState& newState) const
{
    double ret = 0.0;
    ret += delta(oldState.constructionSiteCount, newState.constructionSiteCount) * 0.01;
    ret += delta(oldState.buildingCount, newState.buildingCount) * 0.1;
    return ret;
}

MapPoint Environment::GetNextPOI()
{
    auto& aii = engine_->AII();

    // Take from the already calculated POIs
    while (!poi.buildLocations.empty()) {
        MapPoint ret = poi.buildLocations.back();
        poi.buildLocations.pop_back();

        // still valid?
        BuildingQuality bq = world_->GetBQ(ret, agentId_);
        if (bq > BuildingQuality::Nothing) {
            RoadBuilder roadsPreview(aii, ret, bq);
            if (roadsPreview.CanConnectToAFlag(ret))
                return ret;
        }
    }

    // do not calculate more POIs when we already have too many building sites
    // @todo: what is too many?
    if (aii.GetBuildingSites().size() > 5 * aii.GetStorehouses().size())
        return MapPoint();

    // calculate next POIS
    BuildLocations buildLocations(aii);
    for (const auto* sh : aii.GetStorehouses())
        buildLocations.Calculate(sh->GetFlagPos(), false);
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
