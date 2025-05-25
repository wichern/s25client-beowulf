
#include <mlpack.hpp>
#include "Environment.h"

int main()
{
    std::cout << "Hello World" << std::endl;

    mlpack::rl::TrainingConfig config;
    config.ExplorationSteps() = 1000;
    config.StepLimit() = 20;

    beowulf::Environment env;

    mlpack::rl::FFN<mlpack::MeanSquaredError, mlpack::GaussianInitialization> actorNetwork;
    actorNetwork.Add(new mlpack::Linear(env.StateSize()));
    actorNetwork.Add(new mlpack::ReLU());
    actorNetwork.Add(new mlpack::Linear(1));
    actorNetwork.Add(new mlpack::TanH());

    mlpack::rl::FFN<> criticNetwork;
    criticNetwork.Add(new mlpack::Linear(9 + 1)); // state + action
    criticNetwork.Add(new mlpack::ReLU());
    criticNetwork.Add(new mlpack::Linear(64));

    mlpack::rl::RandomReplay<beowulf::Environment> replayMethod(32, 10000);

    mlpack::rl::SAC<beowulf::Environment, decltype(actorNetwork), decltype(criticNetwork), ens::AdamUpdate> agent(config, actorNetwork, criticNetwork, replayMethod, ens::AdamUpdate(), ens::AdamUpdate(), env);

    agent.Episode(); // Episodes
}
