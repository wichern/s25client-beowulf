// Copyright (C) 2005 - 2024 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GlobalGameSettings.h"

#include <memory>
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

    //void AddPlayer()

    GameObjective getObjective() { return ggs_.objective; }
    void setObjective(GameObjective objective) { ggs_.objective = objective; }
    
    void ActivateReplay(bool activate);

    void Start();
    void Step();
    void Stop();

    bool saveReplay_ = false;

private:
    std::string map_;
    GlobalGameSettings ggs_;
    std::unique_ptr<HeadlessGame> headlessGame_;
};

// virtual base class for a player.
// We have a custom subclass for AIJH.
// The player can create a custom subclass in python
class Player
{
public:
    Player(const std::string& name);
    ~Player();
};

class PlayerAIJH : public Player
{
public:
    PlayerAIJH(const std::string& name);
    ~PlayerAIJH();
};

}  // namespace s25py
