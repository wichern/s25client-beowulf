#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <mlpack/methods/ann/init_rules/random_init.hpp>

using namespace mlpack;
using namespace mlpack::ann;
using namespace arma;

// Parameters
const size_t featureSize = 16;
const double gamma = 0.99;
const double learningRate = 0.001;
const double epsilon = 0.1;

// Define model: simple feedforward NN
FFN<MeanSquaredError<>, RandomInitialization> model;
model.Add<Linear<>>(featureSize, 32);
model.Add<ReLU<>>();
model.Add<Linear<>>(32, 1);

// Optimizer
ens::Adam optimizer(learningRate, 32, 0.9, 0.999, 1e-8, 50000, 1e-5, true);

// Training loop
for (int episode = 0; episode < 1000; ++episode)
{
    // 1. Get candidate location features for current step
    std::vector<vec> candidates = GetCandidateLocations();  // each vec is [featureSize x 1]

    // 2. Stack them into matrix for batch prediction
    mat input(featureSize, candidates.size());
    for (size_t i = 0; i < candidates.size(); ++i)
        input.col(i) = candidates[i];

    // 3. Predict scores
    mat output;
    model.Predict(input, output);  // output: [1 x num_candidates]

    // 4. Choose action (greedy or epsilon-greedy)
    size_t chosenIdx;
    if (math::Random() < epsilon)
        chosenIdx = math::RandInt(candidates.size());  // explore
    else
        chosenIdx = output.index_max();  // exploit

    vec state = candidates[chosenIdx];

    // 5. Take action and get reward and next candidates
    double reward;
    std::vector<vec> nextCandidates;
    std::tie(reward, nextCandidates) = StepEnvironment(chosenIdx);

    // 6. Estimate target Q-value
    double target = reward;

    if (!nextCandidates.empty())  // if not terminal
    {
        mat nextInput(featureSize, nextCandidates.size());
        for (size_t i = 0; i < nextCandidates.size(); ++i)
            nextInput.col(i) = nextCandidates[i];

        mat nextOutput;
        model.Predict(nextInput, nextOutput);
        double maxNextQ = nextOutput.max();  // Q(s', a')
        target += gamma * maxNextQ;
    }

    // 7. Train model to fit Q(state) → target
    mat singleInput = state;
    singleInput.reshape(featureSize, 1);

    mat targetOutput = mat(1, 1).fill(target);

    model.Train(singleInput, targetOutput, optimizer);
}
