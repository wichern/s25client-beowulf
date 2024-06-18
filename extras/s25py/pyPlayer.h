// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "pyBuilding.h"

#include <limits>
#include <string>
#include <vector>

class AIPlayer;

namespace s25py {

class PyGame;

// Base class for AI players
// The base class will only be a dummy (do nothing)
class PyPlayer
{
public:
    PyPlayer(const std::string& name);
    ~PyPlayer();

    virtual void RunGF(unsigned gf, bool gfisnwf);

    std::vector<PyBuilding> GetHeadquaters() const;

protected:
    std::string name_;
    unsigned id_ = std::numeric_limits<unsigned>::max();
    PyGame* game_ = nullptr;

    // Pointer to the in-game AI player object
    AIPlayer* player_ = nullptr;

    friend class PyGame;
};

} // namespace s25py
