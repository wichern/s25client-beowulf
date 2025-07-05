#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "types.h"

#include <chrono>
#include <vector>
#include <string>

namespace beowulf {

class Environment;

/// @brief Observe the training
class Observer
{
public:
    // @todo: does probably not have to be a singleton. can be passed to env instead.
    static Observer& getInstance();

    void init(unsigned maxGf, beowulf::Environment* env);

    void addEpisodeResult(double reward, double epsilon);
    void setCurrentGf(unsigned gf) { currentGf_ = gf; }

    void printState();

private:
    unsigned maxGf_ = 0u;
    beowulf::Environment* env_ = nullptr;

    std::vector<double> rewards_;
    std::vector<double> epsilons_;
    unsigned currentEpisode_ = 1u;
    unsigned chartHeight_ = 10u;
    unsigned currentGf_;
    unsigned lastHeight_ = 0u;
    std::chrono::time_point<std::chrono::steady_clock> trainingStart_;
    std::chrono::time_point<std::chrono::steady_clock> lastFrame_;
    std::vector<std::string> linesReward_;
    std::vector<std::string> linesEpsilon_;

    std::vector<std::string> printChart(const std::vector<double> values, unsigned height, unsigned width) const;
};

} // namespace beowulf
