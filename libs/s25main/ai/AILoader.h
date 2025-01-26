// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "s25util/Singleton.h"
#include "commonDefines.h"
#include <boost/filesystem.hpp>

class AILoader : public Singleton<AILoader>
{
public:
    AILoader();
    ~AILoader();

    void Load();

private:
    void Add(const bfs::path& dir);
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define AILOADER AILoader::inst()
