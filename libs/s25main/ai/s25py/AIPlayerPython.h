// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIPlayer.h"
#include "ai/AILoader.h"

namespace s25py {

class AIPlayerPython final : public AIPlayer
{
public:
    AIPlayerPython(unsigned char playerId, const GameWorldBase& gwb, AI::Level level, unsigned pythonIdx);
    ~AIPlayerPython() override;

    void RunGF(unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg) override;

private:
    py::object py_;
};

} // namespace s25py

