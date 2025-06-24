// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Environment.h"
#include "HeadlessGame.h"
#include "gameData/BuildingProperties.h"
#include "gameTypes/GameTypesOutput.h"
#include "PointOutput.h"
#include "EventManager.h"
#include "Settings.h"
#include "Observer.h"
#include "AsciiMap.h"
#include "ai/AIInterface.h"
#include "FindWhConditions.h"
#include "buildings/nobHQ.h"

namespace beowulf {

Environment::Environment(Settings* settings)
: settings_(settings)
{

}

Environment::State Environment::InitialSample()
{
    RTTR_Assert(settings_);

    engine_ = std::make_unique<HeadlessGame>(settings_);
    world_ = &engine_->world_;
    
    for (unsigned i = 0u; i < world_->GetNumPlayers(); ++i) {
        if (world_->GetPlayer(i).aiInfo.type == AI::Type::Beowulf) {
            agentId_ = i;
            break;
        }
    }

    Environment::State ret = Environment::State(this);
    ret.Update();
    Observer::getInstance().setNextActionParam(AgentActionParamType::Action);
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
    nextState = GameState(this);
    
    bool actionFinished = true;
    if (state.poi_.isValid())
        actionFinished = HandleAction(state, action, nextState.actionStep_, nextState.actionParams_);
    else
    {
        nextState.actionStep_ = 0u;
        nextState.actionParams_.fill(0u);
        // @todo: try to find other poi
    }

    MetaState oldMetaState = ExtractMetaState();

    // advance state
    if (actionFinished)
    {
        engine_->RunNextNWGF();
        beowulf::Observer::getInstance().setCurrentGf(engine_->em_.GetCurrentGF());
        beowulf::Observer::getInstance().printState();
    }
    // @todo: if action was 'NoAction' then we go to another POI
    nextState.Update();

    MetaState nextMetaState = ExtractMetaState();

    // Evaluate reward
    double ret = 0.0;

    ret += RewardGameState(oldMetaState, nextMetaState);
    ret += RewardNewGoods(oldMetaState, nextMetaState);
    ret += RewardNewConnections(oldMetaState, nextMetaState);
    ret += RewardNewBuildings(oldMetaState, nextMetaState);

    // update observer state
    if (actionFinished)
        Observer::getInstance().setNextActionParam(AgentActionParamType::Action);
    else
        Observer::getInstance().setNextActionParam(ACTION_PARAMS[AgentAction(nextState.actionParams_[0])][nextState.actionStep_ - 1]);

#ifdef DEBUG_OUTPUT
    // Print Ascii Map of POI
    auto const& player = world_->GetPlayer(agentId_);
    AsciiMap debug(*world_, player.GetHQPos(), POI_RADIUS+2);
    debug.drawPlayer(agentId_);
    debug.write();
#endif

    return ret;
}

MapPoint Environment::toPoint(const State& state, unsigned point_idx) const
{
    // convert the index to a map point
    if (point_idx == 0)
        return state.poi_;

    unsigned idx = point_idx;
    MapPoint curStartPt = state.poi_;
    for(unsigned r = 1; r <= POI_RADIUS; ++r)
    {
        curStartPt = world_->GetNeighbour(curStartPt, Direction::West);
        MapPoint curPt = curStartPt;
        for(const auto dir : helpers::enumRange(Direction::NorthEast))
        {
            for(unsigned step = 0; step < r; ++step)
            {
                if (--idx == 0u)
                {
                    return curPt;
                }
                curPt = world_->GetNeighbour(curPt, dir);
            }
        }
    }

    RTTR_Assert(false);
    return MapPoint();
}

bool Environment::HandleAction(const State& state, const Action& action, unsigned& nextStep, std::array<unsigned, 4>& nextParams)
{
    // currently no action?
    if (0u == state.actionStep_)
    {
        RTTR_Assert(action.action <= helpers::MaxEnumValue_v<AgentAction>);

        if (AgentAction::NoAction == action.action) {
            nextStep = 0u;
            return true; // no action finished
        }

        nextStep = 1u;  // go to first param selection
        nextParams.fill(0u);
        nextParams[0] = action.action;
        return false; // action not finished
    }

    nextParams = state.actionParams_;

    // we already have an action
    const unsigned paramIdx = state.actionStep_ - 1;
    const AgentAction actionType = AgentAction(state.actionParams_[0]);

    switch (ACTION_PARAMS[actionType][paramIdx]) {
        case AgentActionParamType::Point:
        {
            RTTR_Assert(action.action < GameState::point_count);
            nextParams[state.actionStep_] = action.action;
            nextStep = state.actionStep_ + 1;
        } break;
        case AgentActionParamType::Direction:
        {
            RTTR_Assert(action.action <= helpers::MaxEnumValue_v<Direction>);
            nextParams[state.actionStep_] = action.action;
            nextStep = state.actionStep_ + 1;
        } break;
        case AgentActionParamType::BuildingType:
        {
            // We get a building type from 0 (HeadQuaters) to maxEnumValue(BuildingType) - NUM_UNUSED_BLD_TYPES
            // therefore we need to skip every unused building
            unsigned buildingTypeIdxWithoutInvalids = 0u;
            for (unsigned i = 0u; i <= helpers::MaxEnumValue_v<BuildingType> && buildingTypeIdxWithoutInvalids < action.action; ++i)
            {
                BuildingType bt = BuildingType(i);
                if (bt != BuildingType::Headquarters && BuildingProperties::IsValid(bt))
                    buildingTypeIdxWithoutInvalids++;
            }
            RTTR_Assert(buildingTypeIdxWithoutInvalids == action.action);
            nextParams[state.actionStep_] = action.action;
            nextStep = state.actionStep_ + 1;
        } break;
        default:
            // all other cases not currently needed
            RTTR_Assert(false);
            nextStep = 0u;
            break;
    }

    // do we have all required parameters?
    if (nextStep < ACTION_PARAMS[actionType].size())
        return false;  // wait for more parameters

    nextStep = 0u;

    // create game command
    auto& aii = engine_->players_[agentId_]->getAIInterface();
    switch (actionType) {
        case AgentAction::SetFlag:
        {
            MapPoint pt = toPoint(state, nextParams[1]);
            RTTR_Assert(pt.isValid());
            
#ifdef DEBUG_OUTPUT
            std::cout << "SetFlag(" << pt << ")\n";
#endif
            aii.SetFlag(pt);
        } break;
        case AgentAction::DestroyFlag:
        {
            MapPoint pt = toPoint(state, nextParams[1]);
            RTTR_Assert(pt.isValid());
#ifdef DEBUG_OUTPUT
            std::cout << "DestroyFlag(" << pt << ")\n";
#endif
            aii.DestroyFlag(pt);
        } break;
        case AgentAction::ConnectFlagsDefault:
        {
            MapPoint pt1 = toPoint(state, nextParams[1]);
            MapPoint pt2 = toPoint(state, nextParams[2]);
            RTTR_Assert(pt1.isValid());
            RTTR_Assert(pt2.isValid());
#ifdef DEBUG_OUTPUT
            std::cout << "ConnectFlagsDefault(" << pt1 << "," << pt2 << ")\n";
#endif

            std::vector<Direction> route;
            if (pt1 == pt2 || !aii.FindFreePathForNewRoad(pt1, pt2, &route) || route.size() < 2) {
                // @todo: punish this error?
            } else {
                aii.BuildRoad(pt1, false, route);
            }
        } break;
        case AgentAction::DestroyRoad:
        {
            MapPoint pt = toPoint(state, nextParams[1]);
            RTTR_Assert(pt.isValid());
            Direction dir = static_cast<Direction>(nextParams[2]);
#ifdef DEBUG_OUTPUT
            std::cout << "DestroyRoad(" << pt << "," << dir << ")\n";
#endif
            aii.DestroyRoad(pt, dir);
        } break;
        case AgentAction::SetBuildingSite:
        {
            MapPoint pt = toPoint(state, nextParams[1]);
            RTTR_Assert(pt.isValid());
            BuildingType type = BuildingType(nextParams[2]);
#ifdef DEBUG_OUTPUT
            std::cout << "SetBuildingSite(" << pt << "," << type << ")\n";
#endif
            aii.SetBuildingSite(pt, type);
        } break;
        case AgentAction::DestroyBuilding:
        {
            MapPoint pt = toPoint(state, nextParams[1]);
            RTTR_Assert(pt.isValid());
#ifdef DEBUG_OUTPUT
            std::cout << "DestroyBuilding(" << pt << ")\n";
#endif
            aii.DestroyBuilding(pt);
        } break;
        default:
            RTTR_Assert(false);
            break;
    }

    return true;
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

} // namespace beowulf
