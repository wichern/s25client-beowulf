// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifndef POI_RADIUS
#define POI_RADIUS 6u    // 14 is the maximum range of a catapult
#endif


//#define DEBUG_OUTPUT

#include "types.h"
#include "helpers/EnumArray.h"
#include "s25util/warningSuppression.h"

#include <array>

namespace beowulf {

#define MAX_PARAMS_PER_ACTION 4 // @todo: make constexpr
using ParamList = std::array<AgentActionParamType, MAX_PARAMS_PER_ACTION>;
constexpr helpers::EnumArray<ParamList, AgentAction> SUPPRESS_UNUSED ACTION_PARAMS = {{
    /* NoAction */              { },
    /* SetFlag */               { AgentActionParamType::Point },
    /* DestroyFlag */           { AgentActionParamType::Point },
    /* ConnectFlagsDefault */   { AgentActionParamType::Point, AgentActionParamType::Point },
    /* DestroyRoad */           { AgentActionParamType::Point, AgentActionParamType::Direction },
    /* SetBuildingSite */       { AgentActionParamType::Point, AgentActionParamType::BuildingType },
    /* DestroyBuilding */       { AgentActionParamType::Point }
}};

}  // namespace beowulf
