// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "GameState.h"
#include "ActionSpace.h"

#include <armadillo>

#include <memory>

class GameWorld;

namespace beowulf {

struct Settings;
class HeadlessGame;

class Environment
{
public:
    using Action = BuildActionSpace;
    using State = GameState;

    Environment(Settings* settings);

    // Get an initial game state
    State InitialSample();
    bool IsTerminal(const State& state) const;

    size_t ActionSize() const;
    size_t StateSize() const;

    // Apply given action and calculate reward for the next state.
    double Sample(const State& state, const Action& action, State& nextState);

    GameWorld* world_ = nullptr;
    unsigned agentId_ = 0u;
    Settings* settings_ = nullptr;
    std::unique_ptr<HeadlessGame> engine_;

    unsigned GetCurrentGf() const;

private:
    struct POI
    {

        std::vector<MapPoint> buildLocations;
        std::vector<MapPoint> wareHouses;
        std::vector<MapPoint> harbours;
        std::vector<MapPoint> militaryBuildings;
        std::vector<MapPoint> enemyMilitaryBuildings;
        std::vector<MapPoint> productionBuildings;
    } poi;

    MapPoint GetNextPOI();

    // state information we need for reward calculation
    struct MetaState
    {
        helpers::EnumArray<unsigned, GoodType> goods;
        helpers::EnumArray<unsigned, Job> people;

        unsigned constructionSiteCount = 0u;
        unsigned buildingCount = 0u;

        unsigned connectedBuildings = 0u;

        bool defeated = false;
    };

    MetaState ExtractMetaState() const;
    double RewardGameState(const MetaState& oldState, const MetaState& newState) const;
    double RewardNewGoods(const MetaState& oldState, const MetaState& newState) const;
    double RewardNewPeople(const MetaState& oldState, const MetaState& newState) const;
    double RewardNewConnections(const MetaState& oldState, const MetaState& newState) const;
    double RewardNewBuildings(const MetaState& oldState, const MetaState& newState) const;

    MapPoint toPoint(const State& state, unsigned point_idx) const;
    Direction toDirection(unsigned direction_idx) const;
};

} // namespace beowulf
