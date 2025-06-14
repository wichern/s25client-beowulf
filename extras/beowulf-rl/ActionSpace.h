// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GameCommands.h"
#include "GameState.h"
#include "helpers/MaxEnumValue.h"
#include "types.h"

#include <armadillo>
#include <algorithm>

/*
 * Actions the agent needs to support:
 * reference: libs/s25main/GameCommands.h
 *
 * ACTION (GC)                                          COMMENT
 * - SetFlag(pt)                                        trivial
 * - DestroyFlag(pt)                                    trivial
 * - BuildRoad(boat_road, route)                        ConnectFlagsStrategyA(pt1, pt2), ConnectFlagsStrategyB(pt1, pt2), ...
 * - DestroyRoad(pt, start_dir)                         trivial
 * - UpgradeRoad(pt, start_dir)                         trivial
 * - ChangeDistribution(distributions[])                V2: split into ChangeDistribution(building_type, percentage)
 * - ChangeBuildOrder(use_custom, build_orders[])       V2: split into SetUseCustomBuildOrder(custom), IncreaseBuildPriority(building_type), DecreaseBuildPriority(building_type)
 * - SetBuildingSite(pt, building_type)                 trivial
 * - DestroyBuilding(pt)                                trivial
 * - SetTroopLimit(pt, rank, count)                     V2: requires an additional int
 * - ChangeTransport(transport_orders[])                V2: split into ChangeTransport(ware_type, prio)
 * - ChangeMilitary(settings[])                         V2: split into RecruitingRatio(val), DefenderStrength(val), ActiveDefenders(val), AttackerRatio(val), InlandOccupation(val), ...
 * - ChangeTools(settings[], orders[])                  V2: split
 * - CallSpecialist(pt, job)                            trivial
 * - Attack(pt, soldiers_count, strong_soldiers)        requires an additional int
 * - SeaAttack(pt, soldiers_count, strong_soldiers)     V2: requires an additional int
 * - SetCoinsAllowed(pt, enabled)                       trivial
 * - SetProductionEnabled(pt, enabled)                  trivial
 * - NotifyAlliesOfLocation(pt)                         not needed(?)
 * - SetInventorySetting(pt, what, inventory_setting)   V2: requires an additional int
 * - SetAllInventorySettings(pt, isJob, states[])       V2: can use SetInventorySetting instead
 * - ChangeReserve(pt, rank, count)                     V2: requires an additional int
 * - CheatArmageddon()                                  not used
 * - Surrender()                                        not used by network(?)
 * - DestroyAll()                                       not used
 * - SuggestPact(player, type, duration)                V3: requires an additional int
 * - AcceptPact(id, type, player)                       V3: requires an additional int
 * - CancelPact(type, player)                           V3
 * - SetShipYardMode(pt, mode)                          trivial
 * - SetTempleProductionMode(pt, mode)                  V3
 * - StartStopExpedition(pt, start)                     V2
 * - StartStopExplorationExpedition(pt, start)          V2
 * - ExpeditionCommand(action, ship_id)                 V2
 * - TradeOverLand(pt, what, count)                     V3: requires an additional int
 * 
 * 
 * We can simplify road construction using a ConnectFlags(pt1, pt2) action.
 * Generally, the goal shall be to limit actions to two ints (e.g. idx of pt1 and idx of pt2 or idx of pt and idx of direction)
 * 
 * mlpack::DQN only supports a single action. That is why we will chain it.
 */

class GameWorldBase;

namespace beowulf {

class ActionSpace
{
public:
    ActionSpace() = default;

    unsigned action = 0u;
    static constexpr size_t size = std::max({
        helpers::MaxEnumValue_v<AgentAction>,
        helpers::MaxEnumValue_v<BuildingType>,
        helpers::MaxEnumValue_v<Direction>,
        static_cast<unsigned>(GameState::point_count)
    });
};

} // namespace beowulf
