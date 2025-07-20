// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Game.h"
#include "ai/AIPlayer.h"

#include <chrono>
#include <limits>
#include <vector>

class GameWorld;
class GlobalGameSettings;
class EventManager;

namespace beowulf {

class Settings;

class HeadlessGame
{
public:
    HeadlessGame(const Settings& settings);

    void Start();

    bool IsFinished() const;

    // Proceed in game until next NWGF
    void RunNextNWGF();

    GamePlayer& AgentPlayer();
    const GamePlayer& AgentPlayer() const;

    AIInterface& AII();
    const AIInterface& AII() const;

    Game game;
    GameWorld& world;
    EventManager& em;

private:
    const Settings& settings_;
    std::vector<std::unique_ptr<AIPlayer>> players_;
};

} // namespace beowulf
