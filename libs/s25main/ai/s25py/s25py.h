// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "pyGame.h"
#include "pyPlayer.h"

namespace s25py {

// Trampoline class to support overriding virtual methods.
class PlayerTrampoline : public PyPlayer
{
public:
    using PyPlayer::PyPlayer; // Inherit the constructors
    void RunGF(unsigned gf, bool gfisnwf) override;
};

std::string version();

} // namespace s25py
