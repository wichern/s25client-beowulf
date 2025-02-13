// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "PyPlayer.h"

#include "ai/AIPlayer.h"
#include "buildings/nobHQ.h"
#include "buildings/nobHarborBuilding.h"

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

namespace s25py {

PyPlayer::PyPlayer() {}

PyPlayer::~PyPlayer() {}

void PyPlayer::RunGF(unsigned gf, bool gfisnwf)
{
    RTTR_UNUSED(gf);
    RTTR_UNUSED(gfisnwf);
}

void PyPlayer::OnChatMessage(unsigned sendPlayerId, ChatDestination, const std::string& msg)
{
    RTTR_UNUSED(sendPlayerId);
    RTTR_UNUSED(msg);
}

const nobBaseWarehouse* PyPlayer::GetHeadquater() const
{
    return aii_->GetHeadquarter();
}

void PlayerTrampoline::RunGF(unsigned gf, bool gfisnwf)
{
      PYBIND11_OVERRIDE_NAME(void,/* Return type */
                      PyPlayer,   /* Parent class */
                      "run_gf",   /* Name of method in python */
                      RunGF,      /* Name of function in C++ */
                      gf, gfisnwf /* Argument(s) */
    );
}

void PlayerTrampoline::OnChatMessage(unsigned sendPlayerId, ChatDestination dest, const std::string& msg)
{
    PYBIND11_OVERRIDE_NAME(void,/* Return type */
                      PyPlayer,   /* Parent class */
                      "on_chat_message",   /* Name of method in python */
                      OnChatMessage,      /* Name of function in C++ */
                      sendPlayerId, dest, msg /* Argument(s) */
    );
}

} // namespace s25py