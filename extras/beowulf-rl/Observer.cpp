#// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Observer.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#include "ascii/ascii.h"
#pragma GCC diagnostic pop

namespace beowulf {

Observer& Observer::getInstance()
{
    static Observer instance;
    return instance;
}

void Observer::printEpisode(double reward, double epsilon)
{
    // @todo: print progress of episode

    // ignore the huge negative rewards from losses
    rewards_.push_back(std::max(reward, 0.0));
    if (rewards_.size() > 50)
        rewards_.erase(rewards_.begin());

    epsilons_.push_back(epsilon * 100.0);
    if (epsilons_.size() > 50)
        epsilons_.erase(epsilons_.begin());

    // scroll up
    if (rewards_.size() > 1) {
        for (unsigned j = 0; j <= 2*chartHeight_ + 5; j++) {
            std::cout << "\033[A\033[2K";
        }
    }

    std::cout << "REWARD" << std::endl;
    ascii::Asciichart asciichart_reward(rewards_);
    asciichart_reward.min(0.0);
    asciichart_reward.max(5.0);
    std::cout << asciichart_reward.height(chartHeight_).Plot();

    std::cout << std::endl;
    std::cout << "EPSILON (exploration vs exploitation ratio)" << std::endl;
    ascii::Asciichart asciichart_epsilon(epsilons_);
    asciichart_epsilon.min(0.0);
    asciichart_epsilon.max(100.0);
    std::cout << asciichart_epsilon.height(chartHeight_).Plot();

    std::cout << "Last reward: " << reward << std::endl;
}

void Observer::printInitialTrainingPhase()
{
    std::cout << "initial training phase" << std::endl;
}

} // namespace beowulf
