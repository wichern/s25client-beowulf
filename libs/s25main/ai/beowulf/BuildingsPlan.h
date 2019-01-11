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
#ifndef BEOWULF_BUILDINGSPLAN_H_INCLUDED
#define BEOWULF_BUILDINGSPLAN_H_INCLUDED

#include "ai/beowulf/BuildingsBase.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/RoadIslands.h"
#include "ai/beowulf/BuildingQualityCalculator.h"

#include "world/NodeMapBase.h"
#include "gameTypes/BuildingQuality.h"

#include <vector>

namespace beowulf {

/*
 * This class adds a layer of planned buildings, flags and roads on top of the
 * buildings object.
 */
class BuildingsPlan : public BuildingsBase, public IBlockingReason, public IRoadProvider
{
public:
    BuildingsPlan(const Buildings& buildings, Island island);
    virtual ~BuildingsPlan() = default;

    void PlanBuilding(const MapPoint& pos, Building* bld);
    void UnplanBuilding(const MapPoint& pos);

    void PlanFlag(const MapPoint& pos);
    void UnplanFlag(const MapPoint& pos);

    void PlanRoad(const MapPoint& pos, const std::vector<Direction>& route);
    void UnplanRoad(const MapPoint& pos, const std::vector<Direction>& route);

    void PlanSegment(const MapPoint& pos, Direction dir);
    void UnplanSegment(const MapPoint& pos, Direction dir);

    void Clear();

    bool IsRoadPossible(
            const GameWorldBase& gwb,
            const MapPoint& pos,
            Direction dir) const override;

    std::vector<MapPoint> GetPossibleJoints(const MapPoint& pos, Island island) const;
    Island GetIsland(const MapPoint& pos) const override;

    const std::vector<Building*>& Get() const override;
    Building* Get(const MapPoint& pos) const override;

    bool HasFlag(const MapPoint& pos) const override;
    bool HasRoad(const MapPoint& pos, Direction dir) const override;
    bool HasBuilding(const MapPoint& pos) const override;
    bool IsFlagConnected(const MapPoint& pos) const override;
    const std::vector<MapPoint>& GetFlags() const override;

    BlockingManner GetBM(const MapPoint& pt) const override;
    bool IsOnRoad(const MapPoint& pt) const override;

    std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, ProductionGroup group, const std::vector<BuildingType>& types) const override;

private:
    struct Node {
        unsigned flag = 0;
        unsigned road[3] = { 0, 0, 0 };
        Building* building = nullptr;
    };
    NodeMapBase<Node> nodes_;

    Island island_;
    const Buildings& buildings_;
    std::vector<Building*> buildingsVec_;
    unsigned firstPlanned_;
    std::vector<MapPoint> flags_;
    std::vector<MapPoint> possibleFlags_;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDINGSPLAN_H_INCLUDED
