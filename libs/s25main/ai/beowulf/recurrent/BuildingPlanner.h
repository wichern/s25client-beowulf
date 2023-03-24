// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Heuristics.h"
#include "ai/beowulf/Types.h"
#include "ai/beowulf/recurrent/RecurrentBase.h"

#include <bitset>
#include <limits>
#include <vector>

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

    struct
    {
        std::vector<Building*> requests;

        // One flag as startposition for build location search.
        MapPoint dest;
        unsigned searches = 0;
    } current_;

    // List building types for which no position could be found.
    // Is cleared once the any nodes building quality changed.
    helpers::EnumArray<bool, BuildingType> blacklist_;

    // First field is idx of dest, second is array of buildings to place.
    std::map<MapPoint, std::vector<Building*>, MapPointComp> requests_;
};

template<typename Score>
bool BuildingPlanner::FindBestPosition(const Building* building, MapPoint& pt, Score scoreFunc,
                                       BuildLocations& locations)
{
    std::vector<double> score_vec;
    double bestScore = -std::numeric_limits<double>::max();

    for(const MapPoint& location : locations.Get(building->GetQuality()))
    {
        score_vec.clear();

        if(!costs_.Score(score_vec, building, location))
            continue;

        double score = scoreFunc(score_vec);
        if(score > bestScore)
        {
            bestScore = score;
            pt = location;
        }
    }

    return bestScore != -std::numeric_limits<double>::max();
}

} // namespace beowulf
