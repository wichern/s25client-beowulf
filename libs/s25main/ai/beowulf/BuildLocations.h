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
#ifndef BEOWULF_BUILDLOCATIONS_H_INCLUDED
#define BEOWULF_BUILDLOCATIONS_H_INCLUDED

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
    BuildLocations(const MapBase& world);
    ~BuildLocations();

    /// Calculate all possible build locations starting from a flag of the road network we want to build for.
    void Calculate(const World& buildings, const MapPoint& start);

    /// Update all build locations around the given position.
    void Update(const World& world, const MapPoint& pos, unsigned radius = 2);

    std::vector<MapPoint> Get(BuildingQuality bq) const;
    BuildingQuality Get(const MapPoint& pos) const;
    std::vector<MapPoint> GetNearest(const MapPoint& pos, unsigned amount, BuildingQuality bq) const;

    /// Get total amount of building quality available.
    unsigned GetSum() const;
    unsigned GetSize() const;

private:
    struct Node
    {
        BuildingQuality bq;
        MapPoint pos;
        Node* next;
        Node* prev;
    };

    void Add(const MapPoint& pos, BuildingQuality bq);
    void Remove(Node* node);
    void Update(const MapPoint& pos, BuildingQuality bq);
    void Free();

    NodeMapBase<Node*> map_;
    Node* first_ = nullptr;
    Node* last_ = nullptr;

    unsigned size_ = 0;
    unsigned sum_ = 0;
    Node* freelist_ = nullptr;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDLOCATIONS_H_INCLUDED
