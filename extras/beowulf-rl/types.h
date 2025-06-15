// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

namespace beowulf {

// Enum of actions the agent can perform
enum AgentAction
{   /* ACTION                   ARGUMENTS */
    NoAction = 0,
    SetFlag,                    /* point */
    DestroyFlag,                /* point */
    ConnectFlagsDefault,        /* point1, point2 */
    DestroyRoad,                /* point, direction */
    SetBuildingSite,            /* point, building_type */
    DestroyBuilding             /* point */
};

constexpr auto maxEnumValue(AgentAction)
{
    return AgentAction::DestroyBuilding;
}

enum class AgentActionParamType : unsigned
{
    Action, /* select next action */
    Point,
    BuildingType,
    Direction,
    Percentage,
    Boolean,
    Job,
    GoodType,
    MilitaryVal,
    MilitaryRank,
    PlayerId
};

constexpr auto maxEnumValue(AgentActionParamType)
{
    return AgentActionParamType::PlayerId;
}

}
