// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/*
 * Returning all possible build locations can be too much in late game and open worlds.
 * Question: Which build locations are important?
 * 
 * Solution #1
 * - Return the N highest rated build locations for each of the optimization parameters:
 *      BuildingQuality, available Resources, distance to warehouse, distance to enemy
*/

#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

#include <vector>
#include <array>

class AIInterface;

namespace beowulf {

class World;

class BuildLocations
{
public:
    BuildLocations(AIInterface& aii);
    ~BuildLocations();

    /// Calculate all possible build locations starting from a flag of the road network we want to build for.
    void Calculate(const MapPoint& start, bool confirm = true);

    std::vector<MapPoint> Get(BuildingQuality minBq = BuildingQuality::Hut) const;
    std::vector<std::pair<MapPoint, BuildingQuality>> GetAll() const { return locations_; }


    /// Get total amount of building quality available.
    unsigned GetSum() const;
    unsigned GetSize() const;

private:
    AIInterface& aii_;

    struct Node
    {
        bool visited = false;
    };

    NodeMapBase<Node> map_;
    std::vector<std::pair<MapPoint, BuildingQuality>> locations_;
};

} // namespace beowulf