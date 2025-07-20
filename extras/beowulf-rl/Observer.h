#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "types.h"

#include "gameTypes/MapCoordinates.h"
#include "gameTypes/BuildingType.h"

#include <chrono>
#include <vector>
#include <string>

#include <armadillo>
#include <boost/filesystem/fstream.hpp>

class GameWorld;

namespace beowulf {

/// @brief Observe the training and create report files
class Observer
{
    enum class TrainingState : uint8_t
    {
        Pretraining = 0,
        Exploration,
        Training,
        Test
    };

public:
    Observer(unsigned explorationSteps, unsigned maxGf);

    void BeginExplorationRun();
    void EndExplorationRun(size_t explorationSteps);
    
    void BeginTrainingRun();
    void EndTrainingRun();

    void BeginTestRun();
    void OnSetBuildingSite(const MapPoint& pt, BuildingType bld);
    void OnQValues(const arma::colvec& qvalues);
    void EndTestRun(double reward, double epsilon, const GameWorld& gwb);

    void SetCurrentGf(unsigned gf) { currentGf_ = gf; }
    void Print(const GameWorld& gwb, unsigned playerId);

private:
    unsigned explorationSteps_;
    unsigned maxGf_;
    std::chrono::time_point<std::chrono::steady_clock> trainingStart_;
    boost::filesystem::path outDir_;
    boost::filesystem::ofstream outResultsCsv_;

    TrainingState state_ = TrainingState::Pretraining;
    unsigned currentExplorationStep_ = 0u;
    unsigned currentGf_ = 0u;

    unsigned testRun_ = 0u;

    struct {
        boost::filesystem::path episodeDir_;
        boost::filesystem::ofstream outEpisodeActions_;
        boost::filesystem::ofstream outQValues_;
    } testrun;

    struct {
        std::vector<double> rewards_;
        std::vector<double> epsilons_;
        unsigned currentEpisode_ = 1u;
        unsigned chartHeight_ = 10u;
        std::chrono::time_point<std::chrono::steady_clock> lastFrame_;
        std::vector<std::string> linesReward_;
        std::vector<std::string> linesEpsilon_;
    } live;

    std::vector<std::string> printChart(const std::vector<double> values, unsigned height, unsigned width) const;
};

} // namespace beowulf
