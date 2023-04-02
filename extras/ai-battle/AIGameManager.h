// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "PlayerInfo.h"
#include "ReplayInfo.h"
#include "Game.h"

#include <memory>
#include <vector>

/**
 * Game Manager for AI Battles
 */
class AIGameManager
{
public:
    /// @brief Constructor
    /// @param createReplay     Whether to create a replay file
    /// @param playerInfos      List of AI players to let fight
    AIGameManager(bool createReplay, const std::vector<PlayerInfo>&& playerInfos);

    /// @brief Start game
    /// @param mapPath          Path to mapfile
    /// @return                 true on success
    bool Start(std::string& mapPath);

    /// @brief Run next GF
    /// @return                 true, unless the game is over
    bool Run(unsigned gflimit);

    /// @brief Stop game
    ///
    /// This method will write the replay file.
    /// @param save             Whether to create a savegame
    void Stop(bool save);

private:
    std::vector<PlayerInfo> playerInfos_;
    std::unique_ptr<ReplayInfo> replayInfo_;
    Game game_;
    std::string filename_;

    /// @brief Initialize replayInfo_ object
    /// @param mapPath          path to map
    /// @param random_init      random seed as used in RANDOM.Init()
    /// @return                 true on success
    bool InitReplay(std::string& mapPath, uint64_t random_init);

    /// Wandelt eine GF-Angabe in eine Zeitangabe um (HH:MM:SS oder MM:SS wenn Stunden = 0)
    std::string FormatGFTime(unsigned gf) const;
};
