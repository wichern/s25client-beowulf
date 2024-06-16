// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pyPlayer.h"

#include <boost/nowide/iostream.hpp>
namespace bnw = boost::nowide;

namespace s25py {

PyPlayer::PyPlayer(const std::string& name) : name_(name)
{
    bnw::cout << "PyPlayer(" << name << ")" << std::endl;
}

PyPlayer::~PyPlayer()
{
    bnw::cout << "~PyPlayer(" << name_ << ")" << std::endl;
}

void PyPlayer::RunGF(unsigned /*gf*/, bool /*gfisnwf*/) {}

} // namespace s25py