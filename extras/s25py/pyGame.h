// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Replay.h"
#include "gameTypes/GameSettingTypes.h"
#include <memory>
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

    void AddPlayer(std::shared_ptr<PyPlayer> player);

    bool Step();

    unsigned getCurrentGF() const;

private:
    void Start();

    std::string mapPath_;
    std::string replayPath_;
    GameObjective objective_;
    unsigned maxGF_;
    unsigned randomSeed_;
    unsigned nwfInterval_;

    std::vector<std::shared_ptr<PyPlayer>> players_;
    Replay replay_;
    Game* game_ = nullptr;
};

} // namespace s25py
