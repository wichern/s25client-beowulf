// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GameCommands.h"
#include "GameState.h"
#include "helpers/MaxEnumValue.h"
#include "types.h"

#include <armadillo>

/*
 * Actions the agent could support: libs/s25main/GameCommands.h
 *
 * For every action we wonder if it is heuristically complex enough to leave the decision to the RL network
 * or if we can implement a simple agent to handle it.
 * 
 * Simple Actions
 * ACTION (GC)                                          COMMENT
 * - SetFlag(pt)                                        done by RoadBuilder
 * - DestroyFlag(pt)                                    done by RoadBuilder
 * - BuildRoad(boat_road, route)                        done by RoadBuilder
 * - DestroyRoad(pt, start_dir)                         done by RoadBuilder
 * - UpgradeRoad(pt, start_dir)                         done by RoadBuilder
 * - CallSpecialist(pt, job)                            trivial
 * 
 * Actions on Build Location
 * ACTION (GC)                                          SIZE
 * - SetBuildingSite(pt, building_type)                 |BuildingType| - NUM_UNUSED_BLD_TYPES
 *
 * Actions on Building
 * - DestroyBuilding(pt)                                1
 * 
 * Actions on Warehouse
 * - SetInventorySetting(pt, what, inventory_setting)   Increase/Decrease (2 * (|GoodType| + |Job|))
 * - SetAllInventorySettings(pt, isJob, states[])       use SetInventorySetting
 * 
 * Actions on Military Buildings
 * - SetTroopLimit(pt, rank, count)                     |rank|*|count|
 * - SetCoinsAllowed(pt, enabled)                       2
 * 
 * Actions on Production Buildings
 * - SetProductionEnabled(pt, enabled)                  2
 * - SetShipYardMode(pt, mode)                          2
 * - SetTempleProductionMode(pt, mode)                  2 (use same values as for SetShipYardMode)
 * 
 * Actions on Enemy Military Buildings
 * - Attack(pt, soldiers_count, strong_soldiers)        2 (strong|not strong) * count
 * - SeaAttack(pt, soldiers_count, strong_soldiers)     2 (strong|not strong) * count
 * 
 * Actions on Harbour
 * - StartStopExpedition(pt, start)                     
 * - StartStopExplorationExpedition(pt, start)          
 * - ExpeditionCommand(action, ship_id)                 
 * 
 * Actions on Global Game State (different network?)
 * - ChangeReserve(pt, rank, count)                     
 * - ChangeDistribution(distributions[])                
 * - ChangeBuildOrder(use_custom, build_orders[])       
 * - ChangeMilitary(settings[])                         
 * - ChangeTransport(transport_orders[])                
 * - ChangeTools(settings[], orders[])                  
 * - SuggestPact(player, type, duration)                
 * - AcceptPact(id, type, player)                       
 * - CancelPact(type, player)                           
 * - TradeOverLand(pt, what, count)                     
 * - CheatArmageddon()                                  
 * - Surrender()                                        
 * - DestroyAll()                                       
 */

class GameWorldBase;

namespace beowulf {

class BuildActionSpace
{
public:
    BuildActionSpace() = default;

    unsigned action = 0u;

    // no-action = 0, 1 ... N = create building site
    static constexpr size_t size = 1 + helpers::MaxEnumValue_v<BuildingType> - NUM_UNUSED_BLD_TYPES;
};

} // namespace beowulf
