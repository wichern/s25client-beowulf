// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "gameTypes/BuildingType.h"

class AIInterface;
class nobBaseWarehouse;

namespace beowulf {

/// @brief Provide build order
///
/// Provides a build order per warehouse.
class BuildOrderAgent
{
public:
    BuildOrderAgent(AIInterface& aii);
    ~BuildOrderAgent();
    
    /// void Train(const noBaseWarehouse* )

    BuildingType GetNext(const nobBaseWarehouse* warehouse, unsigned currentGf);

private:
    AIInterface& aii_;
};

} // namespace beowulf