// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GlobalGameSettings.h"

#include <string>

class Game;

namespace s25py
{

class PyPlayer;

class PyGame
{
public:
    PyGame(const std::string& map);
    ~PyGame();

    void AddPlayer(PyPlayer* player);

    GameObjective getObjective() { return ggs_.objective; }
    void setObjective(GameObjective objective) { ggs_.objective = objective; }
    
    void ActivateReplay(bool activate);

    void Start();
    void Step();
    void Stop();

    bool saveReplay_ = false;

private:
    std::string map_;
    std::vector<PyPlayer*> players_;

    GlobalGameSettings ggs_;
    Game* game_ = nullptr;
};

}  // namespace s25py
