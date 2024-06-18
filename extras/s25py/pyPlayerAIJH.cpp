// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyPlayerAIJH.h"
#include "ai/AIPlayer.h"

namespace s25py {

void PyPlayerAIJH::RunGF(unsigned gf, bool gfisnwf)
{
    if(player_)
    {
        player_->RunGF(gf, gfisnwf);
    }
}

} // namespace s25py