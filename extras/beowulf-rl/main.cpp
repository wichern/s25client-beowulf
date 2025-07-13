// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define MLPACK_USE_SYSTEM_STB
#include <mlpack.hpp>

#include "Environment.h"
#include "Settings.h"
#include "AsciiMap.h"
#include "Observer.h"
#include "GameState.h"
#include "HeadlessGame.h"

#include "QuickStartGame.h"
#include "RTTR_Version.h"
#include "RttrConfig.h"
#include "files.h"
#include "random/Random.h"
#include "s25util/System.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;

// Debug unconnected buildings
// [ ] Why does it build wine economy? Is it enabled by default?
//      make switch for building type in Sample(). Also Köhler and Harbor(?) are based on addons/settings
// [ ] smarter road connections (put a flag every 2 to 3 segments)

// @todo: Read replays for initial training
// @todo: Create replays
// @todo: Make it read a configuration file and train on multiple maps/objectives/settings like so:
//        Per episode, select a different map/objective/setting and make sure that all are used

int main(/*int argc, char** argv*/)
{
    bnw::nowide_filesystem();

    // if (argc < 2)
    // {
    //     bnw::cerr << "usage: " << argv[0] << " CONFIG_FILE" << std::endl;
    //     retrun 1;
    // }

    unsigned random_init = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    try {
        RTTRCONFIG.Init();
        RANDOM.Init(random_init);

        // @todo: Train for all possible combinations of mods/objective?
        beowulf::Settings settings;
        settings.map = "AKARTE08.WLD";
        // @todo: How to know how many AIs to add? Instead let Environment fill map completely and set beowulf and a random position each time
        settings.ais.push_back(AI::Info{AI::Type::Beowulf});
        settings.ais.push_back(AI::Info{AI::Type::Dummy});
        settings.maxGf = 10'000u;

        beowulf::Environment env(&settings);
        
        mlpack::TrainingConfig config;
        config.ExplorationSteps() = 100'000;
        config.IsCategorical() = false;
        config.StepSize() = 0.0005;
        config.DoubleQLearning() = true;
        config.TargetNetworkSyncInterval() = 1000;

        //beowulf::Policy<beowulf::Environment> policy(1.0, 40'0000, 0.1, 0.99);
        mlpack::GreedyPolicy<beowulf::Environment> policy(1.0, 40'0000, 0.1, 0.99);
        mlpack::RandomReplay<beowulf::Environment> replayMethod(128, 800'000);

#if 0
        mlpack::rl::SimpleDQN dqn(
            env.StateSize(),    // input layer
            256,    // hidden layer. too small may underfit, too large may overfit and be slow
            beowulf::BuildActionSpace::size); // output layer
#else
        // @todo: Which initialization function makes most sense for us?
        mlpack::FFN<mlpack::MeanSquaredError, mlpack::GaussianInitialization> network(
            mlpack::MeanSquaredError(),
            mlpack::GaussianInitialization(0, 0.01)
        );
        //network.Add(new mlpack::Linear(env.StateSize()));
        // network.Add(new mlpack::ReLU());
        // network.Add(new mlpack::Linear(128));
        // network.Add(new mlpack::ReLU());
        // network.Add(new mlpack::Linear(64));
        //network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(256));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(128));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(64));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(beowulf::BuildActionSpace::size));
        mlpack::rl::SimpleDQN dqn(network);
#endif
        
        mlpack::QLearning<
            beowulf::Environment,
            decltype(dqn),
            ens::AdamUpdate,
            decltype(policy),
            decltype(replayMethod)
            > buildAgent(
                config,
                dqn,
                policy,
                replayMethod,
                ens::AdamUpdate(),
                std::move(env)
            );

        beowulf::Observer::getInstance().init(config.ExplorationSteps(), settings.maxGf, &buildAgent.Environment());

        while(policy.Epsilon() > 0.0) {
            /*double reward =*/ buildAgent.Episode();
            double epsilon = policy.Epsilon();

            // have we reached the end of the initial exploration phase?
            if (buildAgent.TotalSteps() >= config.ExplorationSteps())
            {
                beowulf::Observer::getInstance().beginEpisode();
                // make a test run
                beowulf::Environment::State state = env.InitialSample();
                double totalReturn = 0.0;
                while (!env.IsTerminal(state))
                {
                    arma::colvec actionValue;
                    dqn.Predict(state.Encode(), actionValue);
                    beowulf::Environment::Action action = policy.Sample(actionValue, true, config.NoisyQLearning());
                    beowulf::Environment::State nextState;
                    totalReturn += env.Sample(state, action, nextState);
                    state = nextState;
                }

                beowulf::Observer::getInstance().addEpisodeResult(totalReturn, epsilon);
            } else
                beowulf::Observer::getInstance().addExplorationResult(buildAgent.TotalSteps());
            beowulf::Observer::getInstance().printState();
        }
    } catch(const std::exception& e)
    {
        bnw::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

