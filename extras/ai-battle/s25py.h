// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GlobalGameSettings.h"
#include "ai/AIPlayer.h"

#include <memory>
#include <limits>
#include <string>

class HeadlessGame;

namespace s25py
{

class Player;
    
class Game
{
public:
    Game(const std::string& map);
    ~Game();

    void AddPlayer(Player* player);
    //Player* AddAIJHPlayer(const std::string& name, )

    GameObjective getObjective() { return ggs_.objective; }
    void setObjective(GameObjective objective) { ggs_.objective = objective; }
    
    void ActivateReplay(bool activate);

    void Start();
    void Step();
    void Stop();

    bool saveReplay_ = false;

private:
    std::string map_;
    std::vector<Player*> players_;

    GlobalGameSettings ggs_;
    std::unique_ptr<HeadlessGame> headlessGame_;
    std::vector<PlayerInfo> ais_;
};

// virtual base class for a player.
// We have a custom subclass for AIJH.
// The player can create a custom subclass in python
class Player
{
public:
    Player(const std::string& name);
    ~Player();

    virtual void on_gameframe(bool gfisnwf);

protected:
    std::string name_;
    unsigned id_ = std::numeric_limits<unsigned>::max();
    Game* game_ = nullptr;

    friend class Game;
};

class PyPlayer : public Player
{
public:
    // Inherit the constructors
    using Player::Player;

    // Trampoline for virtual function
    void on_gameframe(bool gfisnwf) override {
        PYBIND11_OVERRIDE(
            void, /* Return type */
            Player,      /* Parent class */
            on_gameframe,          /* Name of function in C++ (must match Python name) */
            gfisnwf      /* Argument(s) */
        );
    }
};

// class AIJHPlayer
// {
// public:
//     AIJHPlayer(const std::string& name, const AI::Level level);
//     ~AIJHPlayer();
// };

}  // namespace s25py
