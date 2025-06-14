// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Environment.h"
#include "HeadlessGame.h"
#include "gameData/BuildingProperties.h"
#include "gameTypes/GameTypesOutput.h"
#include "PointOutput.h"
#include "Settings.h"
#include "AsciiMap.h"

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

    lastFailedActionConstructions_ = failedActionConstructions_;
    failedActionConstructions_ = 0u;

    return Environment::State(this);
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

    auto const& player = world_->GetPlayer(agentId_);

    // AgentMetaState = ExtractMetaState(state)
    unsigned current_boards = player.GetInventory()[GoodType::Boards];
    unsigned current_stones = player.GetInventory()[GoodType::Stones];
    unsigned current_gold = player.GetInventory()[GoodType::Coins];
    unsigned current_soldiers = player.GetInventory()[Job::Private]; // @todo: add other ranks as well

    unsigned current_construction_sites = player.GetBuildingRegister().GetBuildingSites().size();
    unsigned current_buildings = 0u;
    for(const auto bld : helpers::enumRange<BuildingType>())
        if (BuildingProperties::IsUsual(bld))
            current_buildings += player.GetBuildingRegister().GetBuildings(bld).size();
    
    nextState = state;
    bool runGf = false;

    if (0u == state.actionStep_) {
        if (action.action != AgentAction::NoAction) {
            // In order to prevent spamming we give every action a small penalty.
            ret -= 0.6;
            if (action.action > helpers::MaxEnumValue_v<AgentAction>) {
                failedActionConstructions_++;
            } else {
                nextState.actionParams_[0] = action.action;
                nextState.actionStep_ = 1u; // go to first param selection
                ret += 0.1;
            }
        } else {
            ret += 0.01;
            runGf = true;
        }
    } else {
        const unsigned paramIdx = nextState.actionStep_ - 1;
        const auto actionType = AgentAction(nextState.actionParams_[0]);
        switch (ACTION_PARAMS[actionType][paramIdx]) {
            case AgentActionParamType::Point:
                if (action.action >= GameState::point_count) {
                    nextState.actionStep_ = 0u; // go back to action selection
                    failedActionConstructions_++;
                } else {
                    nextState.actionParams_[nextState.actionStep_] = action.action;
                    nextState.actionStep_++;
                    ret += 0.1;
                }
                break;
            case AgentActionParamType::Direction:
                if (action.action > helpers::MaxEnumValue_v<Direction>) {
                    nextState.actionStep_ = 0u; // go back to action selection
                    failedActionConstructions_++;
                } else {
                    nextState.actionParams_[nextState.actionStep_] = action.action;
                    nextState.actionStep_++;
                    ret += 0.1;
                }
                break;
            case AgentActionParamType::BuildingType:
                if (action.action > helpers::MaxEnumValue_v<BuildingType> || !BuildingProperties::IsValid(BuildingType(action.action))) {
                    nextState.actionStep_ = 0u; // go back to action selection
                    failedActionConstructions_++;
                } else {
                    nextState.actionParams_[nextState.actionStep_] = action.action;
                    nextState.actionStep_++;
                    ret += 0.1;
                }
                break;
            default:
                // all other cases not currently needed
                nextState.actionStep_ = 0u; // go back to action selection
                failedActionConstructions_++;
                break;
        }

        if (nextState.actionStep_ == ACTION_PARAMS[actionType].size()) {
            // create game command
            auto& aii = engine_->players_[agentId_]->getAIInterface();
            switch (actionType) {
                case AgentAction::SetFlag:
                {
                    MapPoint pt = toPoint(nextState, nextState.actionParams_[1]);
                    
#ifdef DEBUG_OUTPUT
                    std::cout << "SetFlag(" << pt << ")\n";
#endif
                    aii.SetFlag(pt);
                } break;
                case AgentAction::DestroyFlag:
                {
                    MapPoint pt = toPoint(nextState, nextState.actionParams_[1]);
#ifdef DEBUG_OUTPUT
                    std::cout << "DestroyFlag(" << pt << ")\n";
#endif
                    aii.DestroyFlag(pt);
                } break;
                case AgentAction::ConnectFlagsDefault:
                {
                    MapPoint pt1 = toPoint(nextState, nextState.actionParams_[1]);
                    MapPoint pt2 = toPoint(nextState, nextState.actionParams_[2]);
#ifdef DEBUG_OUTPUT
                    std::cout << "ConnectFlagsDefault(" << pt1 << "," << pt2 << ")\n";
#endif

                    std::vector<Direction> route;
                    if (pt1 == pt2 || !aii.FindFreePathForNewRoad(pt1, pt2, &route) || route.size() < 2) {
                        ret -= 0.1;
                    } else {
                        aii.BuildRoad(pt1, false, route);
                    }
                } break;
                case AgentAction::DestroyRoad:
                {
                    MapPoint pt = toPoint(nextState, nextState.actionParams_[1]);
                    Direction dir = static_cast<Direction>(nextState.actionParams_[2]);
#ifdef DEBUG_OUTPUT
                    std::cout << "DestroyRoad(" << pt << "," << dir << ")\n";
#endif
                    aii.DestroyRoad(pt, dir);
                } break;
                case AgentAction::SetBuildingSite:
                {
                    MapPoint pt = toPoint(nextState, nextState.actionParams_[1]);
                    BuildingType type = BuildingType(nextState.actionParams_[2]);
#ifdef DEBUG_OUTPUT
                    std::cout << "SetBuildingSite(" << pt << "," << type << ")\n";
#endif
                    aii.SetBuildingSite(pt, type);
                } break;
                case AgentAction::DestroyBuilding:
                {
                    MapPoint pt = toPoint(nextState, nextState.actionParams_[1]);
#ifdef DEBUG_OUTPUT
                    std::cout << "DestroyBuilding(" << pt << ")\n";
#endif
                    aii.DestroyBuilding(pt);
                } break;
                default:
                    RTTR_Assert(false);
                    break;
            }

            runGf = true;
            nextState.actionStep_ = 0u;
        }
    }

    // if (!runGf) {
    //     return ret;
    // }
    (void)runGf;
    
    engine_->RunNextNWGF();
    nextState.Update();

    // penalize loosing
    if (player.IsDefeated())
        ret -= 1.0;
    else if (engine_->game_.IsGameFinished())
        ret += 5.0; // @todo: does this always mean winning?

    //std::cout << "GF: " << engine_->em_.GetCurrentGF() << " (fails: " << failedActionConstructions_ << ")" << std::endl;
#ifdef DEBUG_OUTPUT
    // Print Ascii Map of POI
    AsciiMap debug(*world_, player.GetHQPos(), POI_RADIUS+2);
    debug.drawPlayer(agentId_);
    debug.write();
#endif

    // reward new resources
    // @todo: apply function on some resources (the first 10 boards are very good, the next 90 ok, the rest don't care)
    if (current_boards < player.GetInventory()[GoodType::Boards]) {
        ret += static_cast<double>(player.GetInventory()[GoodType::Boards] - current_boards) * 1.0;
    }
    if (current_stones < player.GetInventory()[GoodType::Stones]) {
        ret += static_cast<double>(player.GetInventory()[GoodType::Stones] - current_stones) * 1.0;
    }
    if (current_gold < player.GetInventory()[GoodType::Coins]) {
        ret += static_cast<double>(player.GetInventory()[GoodType::Coins] - current_gold) * 2.0;
    }
    if (current_soldiers < player.GetInventory()[Job::Private]) {
        ret += static_cast<double>(player.GetInventory()[Job::Private] - current_soldiers) * 2.0;
    }

    // @todo: this encourages building destruction once they are finished.
    // Reward new construction sites and finished buildings
    unsigned construction_sites = player.GetBuildingRegister().GetBuildingSites().size();
    if (current_construction_sites < construction_sites) {
        ret += construction_sites - current_construction_sites * 1.0;
    }
    unsigned buildings = 0u;
    for(const auto bld : helpers::enumRange<BuildingType>())
        if (BuildingProperties::IsUsual(bld))
            buildings += player.GetBuildingRegister().GetBuildings(bld).size();

    if (current_buildings < buildings) {
        ret += buildings - current_buildings * 2.0;
    }

    // @todo: reward a building that has newly been connected to the HQ


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



} // namespace beowulf
