// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Replay.h"
#include "gameTypes/GameSettingTypes.h"

// Disable some warnings thrown by pybind11
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnoexcept"
#include <pybind11/pybind11.h>
namespace py = pybind11;
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnoexcept"

#include <string>
#include <vector>

class Game;

namespace s25py {

class PyPlayer;

class PyGame
{
public:
    PyGame(const std::string& mapPath, std::string replayPath, GameObjective objective, unsigned maxGF,
           unsigned randomSeed, unsigned nwfInterval);
    ~PyGame();

    void AddPlayer(py::object player);

    bool Step();

private:
    void Start();

    std::string mapPath_;
    std::string replayPath_;
    GameObjective objective_;
    unsigned maxGF_;
    unsigned randomSeed_;
    unsigned nwfInterval_;

    std::vector<py::object> players_;
    Replay replay_;
    Game* game_ = nullptr;
};

} // namespace s25py
