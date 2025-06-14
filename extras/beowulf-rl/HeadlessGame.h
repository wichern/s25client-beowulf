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
    HeadlessGame(Settings* settings);

    bool IsFinished(unsigned maxGF) const;
    void RunNextNWGF();

//private:
    Settings* settings_;
    Game game_;
    GameWorld& world_;
    EventManager& em_;
    std::vector<std::unique_ptr<AIPlayer>> players_;
};

} // namespace beowulf
