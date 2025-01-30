// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AIPlayerPython.h"

#include "ai/AILoader.h"

namespace s25py {

AIPlayerPython::AIPlayerPython(const unsigned char playerId, const GameWorldBase& gwb, const AI::Level level, unsigned pythonIdx)
    : AIPlayer(playerId, gwb, level)
    , py(AILOADER.create(pythonIdx))
{
}

void AIPlayerPython::RunGF(unsigned gf, bool gfisnwf)
{
    py
}

void AIPlayerPython::OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg)
{

}

} // namespace s25py