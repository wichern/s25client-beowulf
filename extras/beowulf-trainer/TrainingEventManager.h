// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "EventManager.h"

#include <limits>

class TrainingEventManager : public EventManager
{
public:
    TrainingEventManager(unsigned startGF = 0);

    /// Execute the next event increasing the GF to the events GF
    /// If maxGF is given the event is only executed if its GF is <= maxGF and the new GF will be at most maxGF (even if
    /// no event was executed Returns the number of GFs executed
    unsigned ExecuteNextEvent(unsigned maxGF = std::numeric_limits<unsigned>::max());
};