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
#ifndef BEOWULF_RECURRENT_BUILDINGPLANNER_H_INCLUDED
#define BEOWULF_RECURRENT_BUILDINGPLANNER_H_INCLUDED

#include "ai/beowulf/recurrent/RecurrentBase.h"
#include "ai/beowulf/Types.h"
#include "ai/beowulf/Heuristics.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"

#include <vector>
#include <limits>
#include <bitset>

class AIInterface;

namespace beowulf {

class World;

class BuildingPlanner : public RecurrentBase
{
public:
    BuildingPlanner(Beowulf* beowulf);
    ~BuildingPlanner() override;

    void OnRun() override;

    void Request(Building* building, const MapPoint& regionPt);
    unsigned GetRequestCount(const std::vector<BuildingType>&& types, const MapPoint& regionPt) const;
    unsigned GetRequestCount() const;

private:
    void Search();
    void Execute();

    bool Place(Building* building, BuildLocations& locations);

    template<typename Score>
    bool FindBestPosition(const Building* building, MapPoint& pt, Score scoreFunc, BuildLocations& locations);
    bool FindBestRoute(const MapPoint& start, const MapPoint& goodsDest, std::vector<Direction>& route);

    void OnBuildingNote(const BuildingNote& note) override;
    void OnNodeNote(const NodeNote& note) override;

    BuildingPositionCosts costs_;

    struct {
        std::vector<Building*> requests;

        // One flag as startposition for build location search.
        MapPoint dest;
        unsigned searches = 0;
    } current_;

    // List building types for which no position could be found.
    // Is cleared once the any nodes building quality changed.
    std::bitset<NUM_BUILDING_TYPES> blacklist_;

    // First field is idx of dest, second is array of buildings to place.
    std::map<MapPoint, std::vector<Building*>, MapPointComp> requests_;
};

template<typename Score>
bool BuildingPlanner::FindBestPosition(
        const Building* building,
        MapPoint& pt,
        Score scoreFunc,
        BuildLocations& locations)
{
    std::vector<double> score_vec;
    double bestScore = -std::numeric_limits<double>::max();

    for (const MapPoint& location : locations.Get(building->GetQuality())) {
        score_vec.clear();

        if (!costs_.Score(score_vec, building, location))
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

#endif //! BEOWULF_RECURRENT_BUILDINGPLANNER_H_INCLUDED
