// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <armadillo>

namespace beowulf {

class State
{
public:
    State() : data(arma::zeros<arma::colvec>(dimension)) { }

    const arma::colvec& Data() const { return data; }
    arma::colvec& Data() { return data; }

    // int PlayerTurn() const { return playerTurn; }
    // void SwitchPlayer() { playerTurn *= -1; }
  
    // bool IsFull() const {
    //     return arma::all(data != 0);
    // }
    const arma::colvec& Encode() { return data; }

    static constexpr size_t dimension = 9;

private:
    arma::colvec data;  // 9 cells, -1 for O, +1 for X, 0 for empty
    //int playerTurn;     // 1 or -1
};

} // namespace beowulf
