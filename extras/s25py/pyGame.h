// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "Replay.h"
#include "gameTypes/GameSettingTypes.h"
#include "PlayerInfo.h"
#include <memory>
#include <string>
#include <vector>

class Game;
class AIPlayer;

namespace s25py {

class PyPlayer;

class PyGame
{
public:
    PyGame(const std::string& mapPath, std::string replayPath, GameObjective objective, 
           unsigned randomSeed, unsigned nwfInterval);
    ~PyGame();

    void AddPlayerObject(const std::string& name, std::shared_ptr<PyPlayer> player);
    void AddPlayer(const std::string& name, AI::Type type, AI::Level level);
    // @todo: allow specifying python player in RTTR_RTTR/assets by name

    void Run(unsigned maxGF);

    unsigned getCurrentGF() const;
    std::vector<unsigned> getPlayerBuildings() const;

private:
    void Step();
    void Start();

    std::string mapPath_;
    std::string replayPath_;
    GameObjective objective_;
    unsigned randomSeed_;
    unsigned nwfInterval_;

    std::vector<std::pair<PlayerInfo, std::shared_ptr<PyPlayer>>> playerInfos_;
    Replay replay_;
    Game* game_ = nullptr;
};

} // namespace s25py
