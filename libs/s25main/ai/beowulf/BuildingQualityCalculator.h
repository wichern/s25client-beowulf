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
#ifndef BEOWULF_BUILDINGQUALITYCALCULATOR_H_INCLUDED
#define BEOWULF_BUILDINGQUALITYCALCULATOR_H_INCLUDED

#include "world/GameWorldBase.h"
#include "world/BQCalculator.h"
#include "world/NodeMapBase.h"
#include "gameTypes/MapCoordinates.h"

class AIInterface;

namespace beowulf {

class IBlockingReason
{
public:
    virtual BlockingManner GetBM(const MapPoint& pt) const = 0;
};

class IRoadProvider // @todo: stupid name
{
public:
    virtual bool IsOnRoad(const MapPoint& pt) const = 0;
};

class BuildingQualityCalculator : public BQCalculator
{
public:
    BuildingQualityCalculator(const AIInterface& aii);
    BuildingQualityCalculator(const BuildingQualityCalculator& other);

    void AddBlockingReason(IBlockingReason* reason);
    void RemoveBlockingReason(IBlockingReason* reason);

    void AddRoadProvider(IRoadProvider* provider);
    void RemoveRoadProvider(IRoadProvider* provider);

    BuildingQuality GetBQ(const MapPoint& pt) const;

private:
    BlockingManner GetBM(const MapPoint& pt) const override;
    bool IsOnRoad(const MapPoint& pt) const;

    const AIInterface& aii_;
    std::vector<IBlockingReason*> bmProvider_;
    std::vector<IRoadProvider*> roadProvider_;

//    /// For performance reasons, we buffer results of edge checks
//    class CheckedEdges : public NodeMapBase<char>
//    {
//    public:
//        CheckedEdges(const MapExtent& extend);
//        bool GetPathConditionRoad(const GameWorldBase& gwb,
//                                  const MapPoint& pt,
//                                  Direction dir,
//                                  const MapPoint& dest);
//    } edgeChecker_;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDINGQUALITYCALCULATOR_H_INCLUDED
