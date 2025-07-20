// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BuildOrderAgent.h"

#include "buildings/nobBaseWarehouse.h"
#include "ai/AIInterface.h"

#include <queue>

namespace beowulf {

static std::queue<std::pair<unsigned, BuildingType>> S_fixedOrder;

BuildOrderAgent::BuildOrderAgent(AIInterface& aii)
: aii_(aii)
{
    S_fixedOrder = std::queue<std::pair<unsigned, BuildingType>>{{
        { 0, BuildingType::Sawmill },
        { 0, BuildingType::Woodcutter },
        { 0, BuildingType::Woodcutter },
        { 0, BuildingType::Quarry }
    }};
}

BuildOrderAgent::~BuildOrderAgent()
{

}

BuildingType BuildOrderAgent::GetNext(const nobBaseWarehouse* warehouse, unsigned currentGf)
{
    (void)warehouse; // @todo: Implement per warehouse build orders
    // @todo: Consider case where there was not enough build location left to place this.

    if (S_fixedOrder.empty())
        return BuildingType::Nothing2;

    const auto& front = S_fixedOrder.front();
    if (front.first <= currentGf) {
        BuildingType ret = front.second;
        S_fixedOrder.pop();
        return ret;
    }

    return BuildingType::Nothing2;
}

} // namespace beowulf