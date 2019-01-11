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
#ifndef BEOWULF_BUILDINGSBASE_H_INCLUDED
#define BEOWULF_BUILDINGSBASE_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/GameWorldBase.h"

#include <vector>

namespace beowulf {

class Building;

class BuildingsBase
{
public:
    virtual ~BuildingsBase() = default;

    virtual bool HasFlag(const MapPoint& pos) const = 0;
    virtual bool HasRoad(const MapPoint& pos, Direction dir) const = 0;
    virtual bool HasBuilding(const MapPoint& pos) const = 0;
    virtual bool IsFlagConnected(const MapPoint& pos) const = 0;
    virtual const std::vector<MapPoint>& GetFlags() const = 0;
    virtual const std::vector<Building*>& Get() const = 0;
    virtual Building* Get(const MapPoint& pos) const = 0;
    virtual Island GetIsland(const MapPoint& pos) const = 0;
    virtual std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, ProductionGroup group, const std::vector<BuildingType>& types) const = 0;

    // Assumes that we could build a road to pos.
    virtual bool IsRoadPossible(
            const GameWorldBase& gwb,
            const MapPoint& pos,
            Direction dir) const;

    std::pair<MapPoint, unsigned> GetNearestBuilding(
            const MapBase& world,
            const MapPoint& pos,
            const std::vector<BuildingType>& types,
            Island island) const;

    Building* GetGoodsDest(
            const MapBase& world,
            const Building* building,
            Island island,
            const MapPoint& pos) const;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDINGSBASE_H_INCLUDED
