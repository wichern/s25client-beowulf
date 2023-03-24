// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

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
