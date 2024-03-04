// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "TrainingEventManager.h"
#include "Game.h"
#include "gameTypes/AIInfo.h"
#include "ai/AIPlayer.h"
#include "Replay.h"

#include <boost/filesystem.hpp>

#include <vector>
#include <limits>

class GameWorld;

class TrainingGame
{
public:
    TrainingGame(const boost::filesystem::path& map, const std::vector<AI::Info>& ais);
    ~TrainingGame();

    void Run(unsigned maxGF = std::numeric_limits<unsigned>::max());
    void Close();

    void StartReplay(const boost::filesystem::path& path, unsigned random_init);
    void SaveGame(const boost::filesystem::path& savegame) const;

private:
    const boost::filesystem::path& map_;
    Game game_;
    GameWorld& world_;
    TrainingEventManager& em_;
    std::vector<std::unique_ptr<AIPlayer>> players_;

    Replay replay_;
};
