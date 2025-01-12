// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

class GameWorld;

class AIBuildLocations
{
public:
    AIBuildLocations(const GameWorld& world);
    ~AIBuildLocations();

    /// Calculate all possible build locations starting from a flag of the road network we want to build for.
    void Calculate(const MapPoint& start);

    /// Update all build locations around the given position.
    void Update(const MapPoint& pos, unsigned radius = 2);

    std::vector<MapPoint> Get() const;
    std::vector<MapPoint> Get(BuildingQuality bq) const;
    BuildingQuality Get(const MapPoint& pos) const;
    std::vector<MapPoint> GetNearest(const MapPoint& pos, BuildingQuality bq, unsigned amount) const;

    /// Get total amount of building quality available.
    unsigned GetSum() const;
    unsigned GetSize() const;

private:
    struct Node
    {
        BuildingQuality bq;
        MapPoint pos;
    };

    const GameWorld& world_;
    NodeMapBase<Node*> map_;
    std::list<Node> nodes_;

    // @todo: why not use std::list?

    void Add(const MapPoint& pos, BuildingQuality bq);

    unsigned size_ = 0;
    unsigned sum_ = 0;
    Node* freelist_ = nullptr;
    MapPoint regionPt;
};
