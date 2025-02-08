// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "s25util/Singleton.h"
#include <string>

#include <pybind11/pybind11.h>

/// @brief Class to load AI players from Python files
class AILoader : public Singleton<AILoader>
{
public:
    AILoader() = default;
    ~AILoader() = default;

    // Load all AI players from the AI directory.
    void Load();

    // Get the number of loaded AI players.
    unsigned Count() const { return ais_.size(); }

    // Get the name of the AI player at the given index.
    const std::string& GetName(unsigned idx) const { return ais_[idx].name; }

    unsigned GetIdx(const std::string& name) const;

    // Create a pybind11 object of the AI player at the given index.
    pybind11::object Create(unsigned idx);

private:
    struct PyPlayerData
    {
        std::string dir;
        std::string name;
    };

    std::vector<PyPlayerData> ais_;
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define AILOADER AILoader::inst()
