// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once


#include <mlpack.hpp>

#include "gameTypes/JobTypes.h"
#include "gameTypes/GoodTypes.h"

#include "Environment.h"
#include "Observer.h"

namespace beowulf {

template <typename EnvironmentType>
class Policy
{
 public:
  //! Convenient typedef for action.
  using ActionType = typename EnvironmentType::Action;

  /**
   * @param initialEpsilon The initial probability to explore
   *        (select a random action).
   * @param annealInterval The steps during which the probability to explore
   *        will anneal.
   * @param minEpsilon Epsilon will never be less than this value.
   * @param decayRate How much to change the model in response to the
   *        estimated error each time the model weights are updated.
   */
  Policy(const double initialEpsilon,
               const size_t annealInterval,
               const double minEpsilon,
               const double decayRate = 1.0) :
      epsilon(initialEpsilon),
      minEpsilon(minEpsilon),
      delta(((initialEpsilon - minEpsilon) * decayRate) / annealInterval)
  { /* Nothing to do here. */ }

  /**
   * Sample an action based on given action values.
   *
   * @param actionValue Values for each action.
   * @param deterministic Always select the action greedily.
   * @param isNoisy Specifies whether the network used is noisy.
   * @return Sampled action.
   */
  ActionType Sample(const arma::colvec& actionValue,
                    bool deterministic = false,
                    const bool isNoisy = false)
  {
    double exploration = mlpack::Random();
    ActionType action;

    static constexpr helpers::EnumArray<size_t, AgentActionParamType> ACTION_SPACE_SIZE = {{
        /* Action */        helpers::MaxEnumValue_v<AgentAction>,
        /* Point */         GameState::point_count,
        /* BuildingType */  helpers::MaxEnumValue_v<BuildingType> - NUM_UNUSED_BLD_TYPES - 1, // -1 for not allowing HQ
        /* Direction */     helpers::MaxEnumValue_v<Direction>,
        /* Percentage */    100, /* @todo */
        /* Boolean */       2,
        /* Job */           helpers::MaxEnumValue_v<Job>,
        /* GoodType */      helpers::MaxEnumValue_v<GoodType>,
        /* MilitaryVal */   12, /* @todo */
        /* MilitaryRank */  12, /* @todo */
        /* PlayerId */      4 /* @todo */
    }};

    // Select the action randomly.
      size_t actionSpaceSize = ACTION_SPACE_SIZE[Observer::getInstance().getNextActionParam()];
    if (!deterministic && exploration < epsilon && isNoisy == false)
    {
        action.action = static_cast<decltype(action.action)>(mlpack::RandInt(actionSpaceSize));
    }
    // Select the action greedily.
    else
    {
        std::vector<size_t> validActions;
        for (size_t i = 0; i < actionSpaceSize; ++i)
        {
            validActions.push_back(i);
        }

        if (validActions.empty())
        {
          throw std::runtime_error("No valid actions at current state!");
        }

        // Greedy: Choose the max among valid actions only
        double maxVal = -std::numeric_limits<double>::infinity();
        size_t bestIndex = validActions[0];
        for (size_t i : validActions)
        {
          if (actionValue(i) > maxVal)
          {
            maxVal = actionValue(i);
            bestIndex = i;
          }
        }
        action.action = static_cast<decltype(action.action)>(bestIndex);
    }
    return action;
  }

  /**
   * Exploration probability will anneal at each step.
   */
  void Anneal()
  {
    epsilon -= delta;
    epsilon = std::max(minEpsilon, epsilon);
  }

  /**
   * @return Current possibility to explore.
   */
  const double& Epsilon() const { return epsilon; }

 private:
  //! Locally-stored probability to explore.
  double epsilon;

  //! Locally-stored lower bound for epsilon.
  double minEpsilon;

  //! Locally-stored stride for epsilon to anneal.
  double delta;
};

}