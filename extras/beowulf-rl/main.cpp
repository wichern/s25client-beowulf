// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

// now
// Since GameState is default-constructed initially, we need to make the game static?

// somewhen
// @todo: Create replays
// @todo: At end of every episode, print an ASCII map
// @todo: Pre-Train with Replays and AIJH
// @todo: try -opemmp flag
// @todo: multi-thread? one thread for game logic, one for mlpack

//
// usage:
//  beowulf-rl train settings.ini
//

#include <mlpack.hpp>

#include "Environment.h"
#include "Settings.h"
#include "AsciiMap.h"
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

// @todo: make the initial game state size calculation faster by updating Q_Learning to cache the size like SAC already does
// @todo: print Overview with an Observer singleton
//          AsciiMap of current POI
//          Below: graph of episode rewards
//          Graph of failed action constructions in episode
//          Graph of time calculating the episode took
//          -> https://github.com/Civitasv/asciichart
// @todo: clean up Environment::Sample()
// @todo: At end of each episode print all generated commands and the final ascii map of the full map into a file
// @todo: Create replays
// @todo: Improve reward function
// @todo: Make it read a configuration file and train on multiple maps/objectives/settings like so:
//        Per episode, select a different map/objective/setting and make sure that all are used

//void PrintEpisodeResult(unsigned idx, const beowulf::GameState& state, double reward);

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

        // @todo: subclass GreedyPolicy to prefer selecting 0 as action
        //        maybe this allows us to even only select valid actions
        mlpack::GreedyPolicy<beowulf::Environment> policy(1.0, 40'000, 0.1, 0.99);
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
            // @todo: use more POIs
            double reward = agent.Episode();
            bnw::cout << "Episode " << i << ": " << reward <<  std::endl;
            //PrintEpisodeResult(i, agent.State(), reward);
        }
        // @todo: Store network in order to load it again next time.
    } catch(const std::exception& e)
    {
        bnw::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// void PrintEpisodeResult(unsigned idx, const beowulf::GameState& state, double reward)
// {
//     // static bool first_run = true;
//     // if(first_run)
//     //     first_run = false;
//     // else
//     //     printConsole("\x1b[%dA", 8 + world_.GetNumPlayers()); // Move cursor back up

//     AsciiMap debug(state.game_->world_, state.poi_, POI_RADIUS+2);
//     debug.drawPlayer(state.agentId_);
//     debug.write();

//     bnw::cout << "Episode " << idx << ": " << reward << std::endl;
// }

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

