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
#ifndef BEOWULF_BUILDINGPLANNER_H_INCLUDED
#define BEOWULF_BUILDINGPLANNER_H_INCLUDED

#include "ai/beowulf/Types.h"
#include "ai/beowulf/BuildingQualityCalculator.h"
#include "ai/beowulf/BuildingsPlan.h"
#include "ai/beowulf/RoadIslands.h"
#include "ai/beowulf/ResourceMap.h"
#include "ai/beowulf/BuildLocations.h"

#include "ai/AIInterface.h"
#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

#include <vector>

namespace beowulf {

class BuildingPositionCosts;

class BuildingPlanner
{
public:
    BuildingPlanner(
            AIInterface& aii,
            Buildings& buildings,
            ResourceMap& resources);

    ~BuildingPlanner();

    void Init(const std::vector<Building*>& requests, Island island);
    void Search();
    void Execute();
    unsigned GetSearches() const { return searches_; }
    unsigned GetMaxSearches() const { return maxSearches_; }

private:
    AIInterface& aii_;
    Buildings& buildings_;
    ResourceMap& resources_;
    const MapBase& world_;
    BuildingQualityCalculator bqc_;
    BuildLocations locations_;

    BuildingsPlan* plan_ = nullptr;
    Island island_;
    unsigned totalBQ_;
    double bestScore_;
    double lastScore_;
    double temperature_;
    std::vector<Building*> fixedRequests_;
    std::vector<Building*> last_;
    std::vector<Building*> best_;
    unsigned searches_ = 0;
    unsigned maxSearches_;

private:
    void Evaluate(std::vector<Building*>& state);
    void PlaceBuilding(Building* bld, const MapPoint& pos);
    MapPoint FindBestPosition(BuildingPositionCosts& costs, const Building* bld);
    std::vector<Direction> FindBestRoute(BuildingPositionCosts& costs, const MapPoint& start);
};

} // namespace beowulf

#endif //! BEOWULF_PRODUCTIONPLANNER_H_INCLUDED
