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
#ifndef BEOWULF_RECURRENT_PRODUCTIONPLANNER_H_INCLUDED
#define BEOWULF_RECURRENT_PRODUCTIONPLANNER_H_INCLUDED

#include "ai/beowulf/recurrent/RecurrentBase.h"
#include "ai/beowulf/Types.h"
#include "ai/beowulf/Helper.h"

#include "gameTypes/BuildingType.h"
#include "gameTypes/GoodTypes.h"
#include "gameTypes/JobTypes.h"

#include <string>
#include <map>
#include <set>
#include <vector>

class AIInterface;

namespace beowulf {

class Building;

class ProductionPlanner : public RecurrentBase
{
    struct Production
    {
      // Production of all buildings existing, under construction and planned (per minute).
      int produced = 0;

      // Consumed resources (per minute).
      int consumed = 0;
    };

    struct Region {
        Region();
        Region(const Building* building, const MapPoint& flag, bool main = false);

        // Whether this is the main region where we want to produce weapons and beer.
        bool isMain = false;

        MapPoint regionFlag;

        // List of production buildings in this region.
        std::vector<const Building*> buildings;
        std::array<Production, BGD_COUNT> production;

        // Available resources (except for wood and stone, only the resources not yet harvested).
        std::array<unsigned, BResourceCount> resources;

        unsigned GetTotalJobs(Job job) const;
        unsigned GetTotalGoods(GoodType good) const;
        unsigned CountBuildings(const std::vector<BuildingType>&& types, bool constructionFinished = false) const;
    };

public:
    ProductionPlanner(Beowulf* beowulf);

    void OnRun() override;

    unsigned GetTotalProduction(BGoodType type) const;

private:
    void CalculateRegions();
    void CalculateRegion(const MapPoint& flag, std::set<const Building*>& considered);

    void Plan(const MapPoint& regionPt, Region& region);

    void RequestBuilding(const MapPoint& regionPt, BuildingType type, unsigned group = InvalidProductionGroup);

    int GetOvercapacity(const Region& region, BuildingType type, BGoodType good) const;

    std::map<MapPoint, Region, MapPointComp> regions_;
    std::array<Production, BGD_COUNT> globalProduction_;
};

} // namespace beowulf

#endif //! BEOWULF_RECURRENT_PRODUCTIONPLANNER_H_INCLUDED
