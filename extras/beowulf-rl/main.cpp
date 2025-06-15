// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <mlpack.hpp>

#include "Environment.h"
#include "Settings.h"
#include "AsciiMap.h"
#include "Policy.h"
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

// @todo: Run a greedy-only episode after each episode to evalute the current state of the network (test rollout)
//        This value should be used in the reward diagram
// @todo: Implement different Policies (eventually selectable by config)
//          Boltzmann Exploration (Softmax Policy)
//          Upper Confidence Bound (UCB)
// @todo: create log files that include the selected actions for debugging
//          log for rewards, epsilon, etc to be used in discussion
//          log for selected actions (debugging)
//          ascii maps
//          replays
// @todo: Make asciichart display y values with comma and take any container as input
// @todo: make the initial game state size calculation faster by updating Q_Learning to cache the size like SAC already does
// @todo: At end of each episode print all generated commands and the final ascii map of the full map into a file
// @todo: Create replays
// @todo: Move POI
// @todo: Make it read a configuration file and train on multiple maps/objectives/settings like so:
//        Per episode, select a different map/objective/setting and make sure that all are used

#if defined(__MINGW32__) && !defined(__clang__)
void printConsole(const char* fmt, ...) __attribute__((format(gnu_printf, 1, 2)));
#elif defined __GNUC__
void printConsole(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#else
void printConsole(const char* fmt, ...);
#endif

int main(/*int argc, char** argv*/)
{
    bnw::nowide_filesystem();

    // if (argc < 2)
    // {
    //     bnw::cerr << "usage: " << argv[0] << " CONFIG_FILE" << std::endl;
    //     retrun 1;
    // }

    static constexpr unsigned EPISODES = 1000u;

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
        config.ExplorationSteps() = 20'000;
        config.IsCategorical() = false;
        config.StepSize() = 0.0005;
        config.DoubleQLearning() = true;
        config.TargetNetworkSyncInterval() = 1000;

        beowulf::Policy<beowulf::Environment> policy(1.0, 40'0000, 0.1, 0.99);
        mlpack::RandomReplay<beowulf::Environment> replayMethod(128, 100'000);

        // @todo: Which initialization function makes most sense for us?
        mlpack::FFN<mlpack::MeanSquaredError, mlpack::GlorotInitialization> network;
        network.Add(new mlpack::Linear(env.StateSize()));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(128));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(64));
        network.Add(new mlpack::ReLU());
        network.Add(new mlpack::Linear(beowulf::ActionSpace::size));

#if 0
        mlpack::rl::SimpleDQN dqn(
            env.StateSize(),    // input layer
            64,    // hidden layer. too small may underfit, too large may overfit and be slow
            beowulf::ActionSpace::size); // output layer
#else
        mlpack::rl::SimpleDQN dqn(network);
#endif
        
        mlpack::QLearning<
            beowulf::Environment,
            decltype(dqn),
            ens::AdamUpdate,
            decltype(policy),
            decltype(replayMethod)
            > agent(
                config,
                dqn,
                policy,
                replayMethod,
                ens::AdamUpdate(),
                std::move(env)
            );

        for (unsigned i = 0u; i < EPISODES; ++i) {
            /*double reward =*/ agent.Episode();
            double epsilon = policy.Epsilon();

            // have we reached the end of the initial exploration phase?
            if (agent.TotalSteps() >= config.ExplorationSteps())
            {
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

                beowulf::Observer::getInstance().printEpisode(totalReturn, epsilon);
            } else
                beowulf::Observer::getInstance().printInitialTrainingPhase();
        }
    } catch(const std::exception& e)
    {
        bnw::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void printConsole(const char* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    const int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if(len > 0 && (size_t)len < sizeof(buffer))
    {
#ifdef WIN32
        static auto h = setupStdOut();
        WriteConsoleA(h, buffer, len, 0, 0);
#else
        bnw::cout << buffer;
#endif
    }
}

