// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyPlayer.h"

namespace s25py
{
    
PyPlayer::PyPlayer(const std::string& name)
: name_(name)
{
}

PyPlayer::~PyPlayer()
{
}

void PyPlayer::on_gameframe(bool /*gfisnwf*/)
{
}

}  // namespace s25py