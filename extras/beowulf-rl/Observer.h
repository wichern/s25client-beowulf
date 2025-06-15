#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "types.h"

#include <vector>

namespace beowulf {

/// @brief Observe the training
class Observer
{
public:
    static Observer& getInstance();

    // Print the current state:
    //
    // current episode      wall clock
    // ASCII map
    void printEpisode(double reward, double epsilon);

    void printInitialTrainingPhase();

    void setNextActionParam(AgentActionParamType type) { nextActionParamType_ = type; }
    AgentActionParamType getNextActionParam() const { return nextActionParamType_; }

private:
    AgentActionParamType nextActionParamType_ = AgentActionParamType::Action;
    std::vector<double> rewards_;
    std::vector<double> epsilons_;
    unsigned chartHeight_ = 10u;

};

} // namespace beowulf
