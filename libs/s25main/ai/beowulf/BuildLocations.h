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

#include "world/GameWorldBase.h"
#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

#include <vector>
#include <array>

namespace beowulf {

class World;

class BuildLocations
{
public:
    BuildLocations(const GameWorldBase& world, unsigned player);
    ~BuildLocations();

    /// Calculate all possible build locations starting from a flag of the road network we want to build for.
    void Calculate(const MapPoint& start);

    std::vector<MapPoint> Get() const;
    std::vector<MapPoint> Get(BuildingQuality bq) const;
    BuildingQuality Get(const MapPoint& pos) const;

    /// Get total amount of building quality available.
    unsigned GetSum() const;
    unsigned GetSize() const;

private:
    const GameWorldBase& world_;

    struct Node
    {
        BuildingQuality bq;
        //MapPoint pos;
        bool visited = false;
        // Node* next;
        // Node* prev;
    };

    // void Add(const MapPoint& pos, BuildingQuality bq);
    // void Remove(Node* node);
    // void Free();

    NodeMapBase<Node> map_;
    unsigned player_;
    std::vector<std::pair<MapPoint, BuildingQuality>> locations_;
    // Node* first_ = nullptr;
    // Node* last_ = nullptr;

    // unsigned size_ = 0;
    // unsigned sum_ = 0;
    // Node* freelist_ = nullptr;
    // MapPoint regionPt;
};

} // namespace beowulf