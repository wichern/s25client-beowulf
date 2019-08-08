// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/BuildingPlanner.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/CostFunctions.h"

#include "random/Random.h"

#include <limits>
#include <cmath> /* exp() */
#include <algorithm> /* std::min */

namespace beowulf {

BuildingPlanner::BuildingPlanner(
        AIInterface& aii,
        Buildings& buildings,
        ResourceMap& resources)
    : aii_(aii),
      buildings_(buildings),
      resources_(resources),
      world_(aii.gwb),
      locations_(aii.gwb)
{
}

BuildingPlanner::~BuildingPlanner()
{
    buildings_.ClearPlan();
}

size_t factorial(size_t val)
{
    return (val == 1 || val == 0) ? 1 : factorial(val - 1) * val;
}

void BuildingPlanner::Init(const std::vector<Building*>& requests, rnet_id_t rnet)
{
    rnet_ = rnet;

    // Pre-Plan all fixed buildings.
    for (Building* bld : requests) {
        if (bld->GetPt().isValid()) {
            if (canUseBq(buildings_.GetBQC().GetBQ(bld->GetPt()), bld->GetQuality())) {
                buildings_.Plan(bld, bld->GetPt());
                fixedRequests_.push_back(bld);
            }
        } else {
            last_.push_back(bld);
        }
    }

    // Sort requests by quality as an initial permutation.
    std::sort(last_.begin(), last_.end(),
              [](const Building* l, const Building* r)
    {
        return l->GetQuality() < r->GetQuality();
    });

    // Calculate possible build locations.
    locations_.Calculate(buildings_, buildings_.GetFlag(rnet));
    totalBQ_ = locations_.GetSum();

    maxSearches_ = std::min(static_cast<unsigned>(factorial(last_.size())), (unsigned)100);
    searches_ = 1;

    bestScore_ = -std::numeric_limits<double>::max();
    lastScore_ = 0.0;
    temperature_ = 1.0;

    Evaluate(last_);
}

void BuildingPlanner::Search()
{
    // Select two random buildings that should change their order.
    int num = static_cast<int>(last_.size());
    int idx_1 = RANDOM.Rand(__FILE__, __LINE__, 0, num);
    int idx_2 = RANDOM.Rand(__FILE__, __LINE__, 0, (num + idx_1 - 1)) % num;

    std::vector<Building*> state(last_);
    state[idx_1] = last_[idx_2];
    state[idx_2] = last_[idx_1];

    Evaluate(state);

    temperature_ *= 0.95;
    searches_++;
}

void BuildingPlanner::Execute()
{
    // @todo: save best routes
    BuildingPositionCosts costs(aii_, resources_, buildings_);

    for (Building* bld : best_) {
        if (!bld->GetPt().isValid())
            continue;

        buildings_.Construct(bld, bld->GetPt());

        std::vector<Direction> route = FindBestRoute(costs, bld->GetFlag());
        if (!route.empty()) {
            buildings_.ConstructRoad(bld->GetFlag(), route);
        }
    }
}

double HyperVolume(const std::vector<double>& vec) {
    double ret = 1.0;
    for (double v : vec)
        ret *= v;
    return ret;
}

void BuildingPlanner::Evaluate(std::vector<Building*>& state)
{
    BuildingPositionCosts costs(aii_, resources_, buildings_);

    std::vector<MapPoint> places;
    places.resize(state.size());

    for (unsigned  i = 0; i < state.size(); ++i) {
        places[i] = FindBestPosition(costs, state[i]);

        if (!places[i].isValid())
            continue;

        buildings_.Plan(state[i], places[i]);

        MapPoint flagPos = resources_.GetNeighbour(places[i], Direction::SOUTHEAST);
        std::vector<Direction> route = FindBestRoute(costs, flagPos);
        if (!route.empty())
            buildings_.PlanRoad(flagPos, route);

        locations_.Update(buildings_.GetBQC(), flagPos, std::max((size_t)2, route.size()));
    }

    std::vector<double> score_vec;
    unsigned found_positions = 0;
    for (unsigned  i = 0; i < state.size(); ++i) {
        if (!places[i].isValid())
            continue;

        if (costs.Score(score_vec, state[i], places[i], rnet_))
            found_positions++;
    }

    unsigned degradation = totalBQ_ - locations_.GetSum();
    score_vec.push_back(static_cast<double>(degradation));

    double score = static_cast<double>(found_positions) + HyperVolume(score_vec);
    if (score > bestScore_) {
        bestScore_ = score;
        best_ = state;
        lastScore_ = score;
        last_ = state;
    } else {
        double p = exp((lastScore_ - score) / temperature_);
        if (rand() < p) {
            lastScore_ = score;
            last_ = state;
        }
    }

    buildings_.ClearPlan();
}

MapPoint BuildingPlanner::FindBestPosition(
        BuildingPositionCosts& costs,
        const Building* bld)
{
    MapPoint ret = MapPoint::Invalid();

    std::vector<double> score_vec;
    double bestScore = -std::numeric_limits<double>::max();

    for (const MapPoint& location : locations_.Get(bld->GetQuality())) {
        score_vec.clear();

        if (!costs.Score(score_vec, bld, location, rnet_))
            continue;

        double score = HyperVolume(score_vec);
        if (score > bestScore) {
            bestScore = score;
            ret = location;
        }
    }

    return ret;
}

std::vector<Direction> BuildingPlanner::FindBestRoute(
        BuildingPositionCosts& /*costs*/,
        const MapPoint& start)
{
    std::vector<Direction> ret;

    // We might already be connected and we can assume that it is impossible to
    // be connected to another island than the one we want to.
    if (buildings_.IsFlagConnected(start))
        return ret;

    /*
     * We use an A* star to find a path.
     *
     * The search will terminate when it has reached a flag or a position on
     * an existing road where we can put a flag (of the same island).
     */
    bool found = FindPath(start, world_,
    // Condition
    [this, start](const MapPoint& pos, Direction dir)
    {
        if (pos == start && dir == Direction::NORTHWEST)
            return false;

        return buildings_.IsRoadPossible(pos, dir);
    },
    // Cost
    [](const MapPoint& pos, Direction dir)
    {
        /*
         * Costs can be estimated by the simple segment count, but it could
         * also include building quality degradation.
         *
         * The bq degradation is a little complicated, because the cost function
         * only looks at a single segment and not the road walked so far.
         * We can however extract the road walked so far by looking at the
         * came_from map.
         */
        (void)pos;
        (void)dir;
        return 1; // @todo: include bq degradation.
    },
    // End
    [&](const MapPoint& pos)
    {
        // The search can end if we found a way to any flag of the destination
        // island.
        return buildings_.GetRoadNetwork(pos) == rnet_;
    }, &ret);

    if (!found) {
        ret.clear();
    }

    return ret;
}

} // namespace beowulf
