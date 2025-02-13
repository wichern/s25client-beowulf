// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AIPlayerPython.h"
#include "ai/s25py/PyPlayer.h"

namespace py = pybind11;

namespace s25py {

AIPlayerPython::AIPlayerPython(const unsigned char playerId, const GameWorldBase& gwb, const AI::Level level, py::object py)
    : AIPlayer(playerId, gwb, level)
    , py_(py)
{
    std::shared_ptr<s25py::PyPlayer> player = py_.cast<std::shared_ptr<s25py::PyPlayer>>();
    player->aii_ = &aii;
}

void AIPlayerPython::RunGF(unsigned gf, bool gfisnwf)
{
    if (py::hasattr(py_, "run_gf"))
        py_.attr("run_gf")(gf, gfisnwf);
}

void AIPlayerPython::OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg)
{
    if (py::hasattr(py_, "on_chat_message"))
        py_.attr("on_chat_message")(sendPlayerId, msg);
}


} // namespace s25py