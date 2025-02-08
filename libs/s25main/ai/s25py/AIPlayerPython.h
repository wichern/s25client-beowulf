// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIPlayer.h"

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

namespace s25py {

class AIPlayerPython final : public AIPlayer
{
public:
    AIPlayerPython(unsigned char playerId, const GameWorldBase& gwb, AI::Level level, pybind11::object py);
    ~AIPlayerPython() override = default;

    void RunGF(unsigned gf, bool gfisnwf) override;
    void OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg) override;

private:
    pybind11::object py_;
};

} // namespace s25py

