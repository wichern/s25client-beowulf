// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/Beowulf.h"

namespace beowulf {

Beowulf::Beowulf(const unsigned char playerId,
                 const GameWorldBase& gwb,
                 const AI::Level level)
    : AIPlayer(playerId, gwb, level)
{
}

Beowulf::~Beowulf()
{
}

void Beowulf::RunGF(const unsigned gf, bool gfisnwf)
{
    (void)gf;
    (void)gfisnwf;
}

} // namespace beowulf