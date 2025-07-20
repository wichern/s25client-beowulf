// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GlobalGameSettings.h"
#include "gameTypes/AIInfo.h"

#include <boost/nowide/filesystem.hpp>

#include <vector>

namespace beowulf {

struct Settings
{
    GlobalGameSettings ggs;
    std::vector<AI::Info> ais;
    unsigned agentIdx; // index into 'ais' indicating who is the agent to be trained
    boost::filesystem::path map;
    unsigned maxGf;
};

} // namespace beowulf
