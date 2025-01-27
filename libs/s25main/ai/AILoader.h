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

    unsigned Count() const { return ais_.size(); }
    const std::string& GetName(unsigned idx) const { return ais_[idx].name; }

private:
    struct PyPlayerData
    {
        std::string dir;
        std::string name;
    };

    std::vector<PyPlayerData> ais_;
    const bfs::path rootDir_;
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define AILOADER AILoader::inst()
