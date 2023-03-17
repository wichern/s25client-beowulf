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
    AIGameManager(bool createReplay, const std::vector<PlayerInfo>& playerInfos);

    bool Start(std::string& mapPath);
    bool Run();
    void Stop();

private:
    std::vector<PlayerInfo> playerInfos_;
    std::unique_ptr<ReplayInfo> replayInfo_;
    Game game_;

    bool InitReplay(std::string& mapPath, uint64_t random_init);
};
