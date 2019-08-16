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
#ifndef BEOWULF_BUILDINGPLANNERBASE_H_INCLUDED
#define BEOWULF_BUILDINGPLANNERBASE_H_INCLUDED

#include "ai/beowulf/Types.h"
#include "ai/beowulf/CostFunctions.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Building.h"

#include <vector>
#include <limits>

class AIInterface;

namespace beowulf {

class World;
class Resources;

class BuildingPlannerBase
{
public:
    BuildingPlannerBase(
            AIInterface& aii,
            World& buildings,
            Resources& resources,
            rnet_id_t rnet);

    virtual ~BuildingPlannerBase();

    /**
     * @brief Initialize a new planning.
     *
     * @param requests  A list of buildings to be placed.
     */
    virtual void Init(const std::vector<Building*>& requests) = 0;

    /**
     * @brief Run one search.
     *
     * A search can mean one tree traversal in MCTS, one new generation in evolutionary algorithms
     * or a new cooldown in simulated annealing.
     */
    virtual void Search() = 0;

    /**
     * @brief Execute the current best plan.
     *
     * There is no need to run all possible searches.
     * This call will execute the currently best known placement.
     */
    virtual void Execute() = 0;

    /**
     * @brief Number of searches run.
     */
    virtual unsigned GetSearches() const = 0;

    /**
     * @brief Total number of possible searches.
     */
    virtual unsigned GetMaxSearches() const = 0;

protected:
    /**
     * @brief Place given building.
     *
     * If the building already has a valid point set, it will be used.
     * If the fixed point does not work, this method returns 'false'.
     *
     * If the building does not yet have a valid point, the best
     * location will be searched.
     *
     * @param building
     * @param pt
     * @param route
     * @param construct     construct (true) or plan (false)
     * @return
     */
    bool Place(Building* building, MapPoint& pt, std::vector<Direction>& route, bool construct);

    template<typename Score>
    bool FindBestPosition(const Building* building, MapPoint& pt, Score scoreFunc);
    bool FindBestRoute(const  MapPoint& start, const MapPoint& goodsDest, std::vector<Direction>& route);

    AIInterface& aii_;
    World& world_;
    Resources& resources_;
    BuildLocations locations_;
    BuildingPositionCosts costs_;
    rnet_id_t rnet_;
};

template<typename Score>
bool BuildingPlannerBase::FindBestPosition(
        const Building* building,
        MapPoint& pt,
        Score scoreFunc)
{
    std::vector<double> score_vec;
    double bestScore = -std::numeric_limits<double>::max();

    for (const MapPoint& location : locations_.Get(building->GetQuality())) {
        score_vec.clear();

        if (!costs_.Score(score_vec, building, location, rnet_))
            continue;

        double score = scoreFunc(score_vec);
        if (score > bestScore) {
            bestScore = score;
            pt = location;
        }
    }

    return bestScore != -std::numeric_limits<double>::max();
}

} // namespace beowulf

#endif //! BEOWULF_BUILDINGPLANNERBASE_H_INCLUDED
