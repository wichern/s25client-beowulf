// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define MLPACK_USE_SYSTEM_STB
#include <mlpack.hpp>

//#include "Environment.h"
#include "Settings.h"
#include "AsciiMap.h"
#include "Observer.h"
//#include "GameState.h"
#include "HeadlessGame.h"
#include "BuildLocationSelectorTrainer.h"

#include "ai/beowulf/BuildOrderAgent.h"
#include "ai/beowulf/RoadBuilder.h"

#include "QuickStartGame.h"
#include "RTTR_Version.h"
#include "RttrConfig.h"
#include "files.h"
#include "buildings/nobHQ.h"
#include "random/Random.h"
#include "gameData/BuildingConsts.h"
#include "s25util/System.h"
#include "EventManager.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace bnw = boost::nowide;
namespace bfs = boost::filesystem;

std::vector<std::string> Split(const std::string& s, char seperator);
void RunEpisode(const beowulf::Settings& settings, beowulf::Observer& observer, beowulf::BuildLocationSelectorTrainer& buildLocations);
void ConnectBuilding(beowulf::Observer& observer, AIInterface& aii, BuildingType bld, const MapPoint& pt);

// [ ] Implement network updates based on reward
// [ ] Cleanup AsciiMap
//      Think about layering
//      Support colors
//      Remove scaling
// [ ] Create replays

int main(/*int argc, char** argv*/)
{
    bnw::nowide_filesystem();

    unsigned random_init = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    try {
        RTTRCONFIG.Init();
        RANDOM.Init(random_init);

        // open config
        boost::property_tree::ptree iniSettings;
        boost::property_tree::ini_parser::read_ini("beowulf.ini", iniSettings);

        beowulf::Settings settings;
        settings.map = RTTRCONFIG.ExpandPath(Split(iniSettings.get<std::string>("Global.Maps"), ';').front());
        settings.ais.push_back(AI::Info{AI::Type::Beowulf});
        settings.ais.push_back(AI::Info{AI::Type::Dummy});
        settings.agentIdx = 0u;
        settings.maxGf = iniSettings.get<unsigned>("Global.MaxGf");

        beowulf::BuildLocationSelectorTrainer buildLocSelection;

        beowulf::Observer observer(1000, settings.maxGf);

        while (true) {
            RunEpisode(settings, observer, buildLocSelection);
        }
    } catch(const std::exception& e)
    {
        bnw::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#if 0 // Old approach in which we combine location selection and build order
int main(/*int argc, char** argv*/)
{
    bnw::nowide_filesystem();

    // open config
    boost::property_tree::ptree iniSettings;
    boost::property_tree::ini_parser::read_ini("beowulf.ini", iniSettings);

    unsigned random_init = static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    try {
        RTTRCONFIG.Init();
        RANDOM.Init(random_init);

        beowulf::Settings settings;
        settings.map = Split(iniSettings.get<std::string>("Global.Maps"), ';').front();
        settings.ais.push_back(AI::Info{AI::Type::Beowulf});
        settings.ais.push_back(AI::Info{AI::Type::Dummy});
        settings.maxGf = iniSettings.get<unsigned>("Global.MaxGf");
        
        mlpack::TrainingConfig config;
        config.ExplorationSteps() = iniSettings.get<size_t>("QLearning.ExplorationSteps");
        config.IsCategorical() = false;
        config.StepSize() = iniSettings.get<double>("QLearning.StepSize");
        config.DoubleQLearning() = true;
        config.TargetNetworkSyncInterval() = iniSettings.get<size_t>("QLearning.TargetNetworkSyncInterval");

        beowulf::Observer observer(config.ExplorationSteps(), settings.maxGf);
        beowulf::Environment env(&settings, &observer);

        mlpack::GreedyPolicy<beowulf::Environment> policy(
            iniSettings.get<double>("Policy.InitialEpsilon"),
            iniSettings.get<size_t>("Policy.AnnealInterval"),
            iniSettings.get<double>("Policy.MinEpsilon"),
            iniSettings.get<double>("Policy.DecayRate"));
        mlpack::RandomReplay<beowulf::Environment> replayMethod(
            iniSettings.get<size_t>("ReplayMethod.BatchSize"), 
            iniSettings.get<size_t>("ReplayMethod.Capacity"),
            iniSettings.get<size_t>("ReplayMethod.LookaheadSteps"));

#if 0
        mlpack::rl::SimpleDQN dqn(
            env.StateSize(),    // input layer
            256,    // hidden layer. too small may underfit, too large may overfit and be slow
            beowulf::BuildActionSpace::size); // output layer
#else
        mlpack::FFN<mlpack::MeanSquaredError, mlpack::GaussianInitialization> network(
            mlpack::MeanSquaredError(),
            mlpack::GaussianInitialization(0, 0.01)
        );
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

        // @todo: Pretrain Phase
        // for (const auto& replay : replays) {
        //     // Setup environment
        //     for (const auto& action : replay.actions) {
        //         // create state
        //         // extract action (if it is SetBuildingSite)
        //         // create nextState
        //         // calculate reward

                
        //         replayMethod.Store(state, action, reward, nextState,
        //             env.IsTerminal(nextState), config.Discount());

        //         if (end of exploration) {
        //             agent.TrainAgent();
        //         }
        //     }
        // }

        // Exploration Phase
        while (buildAgent.TotalSteps() < config.ExplorationSteps()) {
            observer.BeginExplorationRun();
            buildAgent.Episode();
            observer.EndExplorationRun(buildAgent.TotalSteps());
        }

        // Training Phase
        unsigned trainingRun = 0;
        double minEpsilon = iniSettings.get<double>("Policy.MinEpsilon");
        while(policy.Epsilon() > minEpsilon) {
            observer.BeginTrainingRun();
            buildAgent.Episode();
            observer.EndTrainingRun();

            // after every two trainings, we do a test run
            if (trainingRun % 10 == 0u)
            {
                observer.BeginTestRun();
                beowulf::Environment::State state = env.InitialSample();
                double totalReturn = 0.0;
                while (!env.IsTerminal(state))
                {
                    arma::colvec actionValue;
                    dqn.Predict(state.Encode(), actionValue);
                    observer.OnQValues(actionValue);
                    beowulf::Environment::Action action = policy.Sample(actionValue, true, config.NoisyQLearning());
                    beowulf::Environment::State nextState;
                    totalReturn += env.Sample(state, action, nextState);
                    state = nextState;
                }
                observer.EndTestRun(totalReturn, policy.Epsilon(), env);
            }

            observer.Print(&env);
        }
    } catch(const std::exception& e)
    {
        bnw::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif

std::vector<std::string> Split(const std::string& s, char seperator)
{
    std::vector<std::string> ret;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        ret.push_back(substring);
        prev_pos = ++pos;
    }
    ret.push_back(s.substr(prev_pos, pos-prev_pos));
    return ret;
}


void RunEpisode(const beowulf::Settings& settings, beowulf::Observer& observer, beowulf::BuildLocationSelectorTrainer& buildLocations)
{
    observer.BeginTestRun();
    beowulf::HeadlessGame game(settings);
    game.Start();

    auto& aii = game.AII();
    const auto* hq = aii.GetHeadquarter();
    
    beowulf::BuildOrderAgent buildOrder(aii);

    while (!game.IsFinished()) {
        game.RunNextNWGF();

        BuildingType bld = buildOrder.GetNext(hq, game.em.GetCurrentGF());
        if (!BuildingProperties::IsValid(bld))
            continue;

        bool random = true; // @todo based on eps
        MapPoint destPt = buildLocations.Select(aii, hq, bld, random);
        if (!destPt.isValid())
            continue;

        ConnectBuilding(observer, aii, bld, destPt);
    }

    // Rate final game and train BuildLocationSelector network
    // @todo

    observer.EndTestRun(0.0, 1.0, game.world);
    observer.Print(game.world, settings.agentIdx);
}

void ConnectBuilding(beowulf::Observer& observer, AIInterface& aii, BuildingType bld, const MapPoint& pt)
{
    beowulf::RoadBuilder roads(aii, pt, BUILDING_SIZE[bld]);
    MapPoint flagPos = aii.gwb.GetNeighbour(pt, Direction::SouthEast);

    if (roads.IsConnected(flagPos, true)) {
        aii.SetBuildingSite(pt, bld);
        observer.OnSetBuildingSite(pt, bld);
    } else {
        std::vector<Direction> route;
        std::vector<MapPoint> flags;
        if (roads.FindConnectionToNearestFlag(flagPos, &route, beowulf::RoadBuilder::ShortestTwoSegmentFlags, &flags)) {
            aii.SetBuildingSite(pt, bld);
            aii.BuildRoad(flagPos, false, route);
            for (const MapPoint& flag : flags)
                aii.SetFlag(flag);
            observer.OnSetBuildingSite(pt, bld);
        } else if (roads.FindConnectionToNearestFlag(flagPos, &route, beowulf::RoadBuilder::ShortestAsManyFlagsAsPossible)) {
            aii.SetBuildingSite(pt, bld);
            aii.BuildRoad(flagPos, false, route);
            observer.OnSetBuildingSite(pt, bld);
        } else {
            RTTR_Assert(false); // how come we got this position in the first place?
        }
    }
}
