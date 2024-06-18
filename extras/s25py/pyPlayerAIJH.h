// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "pyPlayer.h"

namespace s25py {

// AI player class wrapping AIJH
class PyPlayerAIJH : public PyPlayer
{
public:
    using PyPlayer::PyPlayer; // inherit constructors
    void RunGF(unsigned gf, bool gfisnwf) override;
};

} // namespace s25py
