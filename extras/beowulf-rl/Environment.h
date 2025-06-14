// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GameState.h"
#include "ActionSpace.h"

#include <armadillo>

#include <memory>

class GameWorld;

namespace beowulf {

struct Settings;
class HeadlessGame;

class Environment
{
public:
    using Action = ActionSpace;
    using State = GameState;

    Environment(Settings* settings);

    // Get an initial game state
    State InitialSample();
    bool IsTerminal(const State& state) const;

    size_t ActionSize() const;
    size_t StateSize() const;

    // Apply given action and calculate reward for the next state.
    double Sample(const State& state, const Action& action, State& nextState);

    GameWorld* world_ = nullptr;
    unsigned agentId_ = 0u;
    Settings* settings_ = nullptr;
    std::unique_ptr<HeadlessGame> engine_;

    unsigned GetCurrentGf() const;

    unsigned lastFailedActionConstructions_ = 0u;
    unsigned failedActionConstructions_ = 0u;
private:

    MapPoint toPoint(const State& state, unsigned point_idx) const;
    Direction toDirection(unsigned direction_idx) const;
};

} // namespace beowulf
