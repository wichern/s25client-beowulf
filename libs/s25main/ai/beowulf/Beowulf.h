// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIPlayer.h"

namespace beowulf {

class Beowulf : public AIPlayer
{
public:
    Beowulf(const unsigned char playerId,
            const GameWorldBase& gwb,
            const AI::Level level);
    ~Beowulf() override;

    void RunGF(const unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned /*sendPlayerId*/, ChatDestination, const std::string& /*msg*/) override {}
};

} // namespace beowulf