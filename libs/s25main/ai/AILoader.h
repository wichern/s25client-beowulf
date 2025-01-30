// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/AIPlayer.h"
#include "s25util/Singleton.h"
#include "commonDefines.h"
#include <boost/filesystem.hpp>

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

// @todo: move to s25py subfolder
class AILoader : public Singleton<AILoader>
{
public:
    AILoader();
    ~AILoader();

    void Load();

    unsigned Count() const { return ais_.size(); }
    const std::string& GetName(unsigned idx) const { return ais_[idx].name; }
    py::object Create(unsigned idx);

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
