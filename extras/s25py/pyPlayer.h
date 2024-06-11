// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>
#include <limits>

namespace s25py
{

class PyGame;

// Base class for AI players
// The base class will only be a dummy (do nothing)
class PyPlayer
{
public:
    PyPlayer(const std::string& name);
    ~PyPlayer();

    virtual void on_gameframe(bool gfisnwf);

protected:
    std::string name_;
    unsigned id_ = std::numeric_limits<unsigned>::max();
    PyGame* game_ = nullptr;

    friend class PyGame;
};

}  // namespace s25py
