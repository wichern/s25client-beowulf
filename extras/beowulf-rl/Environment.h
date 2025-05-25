// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "State.h"

#include <armadillo>

namespace beowulf {

class TicTacToeAction
  {
   public:
    /**
     * Construct an action instance.
     */
    TicTacToeAction() : action(1)
    { /* Nothing to do here */ }
    std::vector<double> action;
    // Storing degree of freedom.
    static const size_t size = 1;
  };

class Environment
{
public:
    using Action = TicTacToeAction;
    using State = beowulf::State;

    Environment() { /* Initialize parameters */ }

    State InitialSample()
    {
        return State();
    }

    bool IsTerminal(const State& state)
    {
        //return CheckWinner(state) != 0 || state.IsFull();
        return true;
    }

    size_t ActionSize() const { return 1; }
    size_t StateSize() const { return 9; }

    double Sample(const State& state,
                  const Action& action,
                  State& nextState)
    {
        nextState = state;
        return -0.1;
    }
};

} // namespace beowulf
