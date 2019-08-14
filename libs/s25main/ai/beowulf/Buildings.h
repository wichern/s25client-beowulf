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
#ifndef BEOWULF_BUILDINGS_H_INCLUDED
#define BEOWULF_BUILDINGS_H_INCLUDED

#include "ai/beowulf/Types.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/RoadIslands.h"
#include "ai/beowulf/BuildingQualityCalculator.h"

#include "ai/AIInterface.h"
#include "gameTypes/MapCoordinates.h"
#include "world/NodeMapBase.h"

#include <vector>
#include <utility> /* pair */

namespace beowulf {

/**
 * @brief The Buildings class contains all buildings and roads the beowulf owns or plans.
 *
 * What makes Buildings especially complex is that while the planner is running, the world can change.
 * Therefore, every change of the world (building/flag/roadsegment added or removed) will clear the plan.
 */
class Buildings : public IBlockingReason, public IRoadProvider
{
public:
    Buildings(AIInterface& aii);
    ~Buildings();

public:
    // Construct
    // Construction requests the engine to place a building/flag/road.
    // The engine can reject these requests.
    void Construct(Building* building, const MapPoint& pt);
    void ConstructFlag(const MapPoint& pt);
    void ConstructRoad(const MapPoint& pt, const std::vector<Direction>& route);

    // Deconstruct
    // Deconstruction requests the engine to remove a building/flag/road.
    void Deconstruct(Building* building);
    void DeconstructFlag(const MapPoint& pt);
    void DeconstructRoad(const MapPoint& pt, const std::vector<Direction>& route);

    // Plan
    // Planning buildings/flags/roads does not result in any engine requests
    // but will alter the return of GetFlag(), HasBuilding(), GetBM() and more.
    void Plan(Building* building, const MapPoint& pt);
    void PlanFlag(const MapPoint& pt); // Ignored if already has plan at 'pt'.
    void PlanRoad(const MapPoint& pt, const std::vector<Direction>& route);
    void ClearPlan();

    // Road Networks
    /// Get the road network id the given point is connected to.
    rnet_id_t GetRoadNetwork(const MapPoint& pt) const;
    /// Get any flag connected to given road network. (Note: not updated while planning.)
    MapPoint GetFlag(rnet_id_t rnet) const;
    const RoadNetworks& GetRoadNetworks() const;

    // Remove (remove the object from our data structure)
    void Remove(Building* building);
    void RemoveFlag(const MapPoint& pt); // you probably want to use DeconstructFlag!
    void RemoveRoad(const MapPoint& pt, const std::vector<Direction>& route);

    // Buildings
    Building* Create(BuildingType type, Building::State state, pgroup_id_t group = InvalidProductionGroup, const MapPoint& pt = MapPoint::Invalid());
    const std::vector<Building*>& Get() const;
    Building* Get(const MapPoint& pt) const;
    bool HasBuilding(const MapPoint& pt) const;
    void SetState(Building* building, Building::State state);
    void SetPos(Building* building, const MapPoint& pt);

    // Flags
    const std::vector<MapPoint>& GetFlags() const;
    bool HasFlag(const MapPoint& pt) const;
    FlagState GetFlagState(const MapPoint& pt) const;
    void SetFlagState(const MapPoint& pt, FlagState state);
    bool IsPointConnected(const MapPoint& pt) const;

    // Roads
    bool HasRoad(const MapPoint& pt, Direction dir) const;
    RoadState GetRoadState(const MapPoint& pt, Direction dir) const;
    void SetRoadState(const MapPoint& pt, const std::vector<Direction>& route, RoadState state);
    //void AddRoadUser(Building* building, const std::vector<Direction>& route);

    const BuildingQualityCalculator& GetBQC() const;
    BlockingManner GetBM(const MapPoint& pt) const override;
    bool IsOnRoad(const MapPoint& pt) const override;

    /// Get building of type 'types' in group 'group' with the shortest flag distance.
    std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, pgroup_id_t group, const std::vector<BuildingType>& types) const;
    const GameWorldBase& GetWorld() const;

    /// Check if we can build a segment from 'pt' in 'dir'. Assumes that we can start at 'pt'.
    bool IsRoadPossible(
            const MapPoint& pt,
            Direction dir) const;

    std::pair<MapPoint, unsigned> GetNearestBuilding(
            const MapPoint& pt,
            const std::vector<BuildingType>& types,
            rnet_id_t rnet) const;

    Building* GetGoodsDest(
            const Building* building,
            rnet_id_t rnet,
            const MapPoint& pt) const;

private:
    void SetRoadState(const MapPoint& pos, Direction dir, RoadState state);
    //void AddRoadUser(const MapPoint& pos, Direction dir, Building* building);
    //void RemoveRoadUser(Building* building);
    //bool HasRoadUser(const MapPoint& pos, Direction dir, Building* building);
    //void RemoveRoadUser(const MapPoint& pos, Direction dir, Building* building);
    bool InsertIntoExistingGroup(Building* building);

private:
    void PlanSegment(const MapPoint& pt, Direction dir);

    struct Node {
        Building* building          = nullptr;
        FlagState flag              = FlagDoesNotExist;
        unsigned flagPlanCount      = 0;
        RoadState roads[3]          = { RoadDoesNotExist, RoadDoesNotExist, RoadDoesNotExist };
        unsigned roadPlanCount[3]   = { 0, 0, 0 };
        //std::vector<Building*> users[3];
    };

    struct Group
    {
        std::vector<BuildingType> types;
        std::vector<Building*> buildings;
    };

    AIInterface& aii_;
    BuildingQualityCalculator bqc_;
    NodeMapBase<Node> nodes_;
    std::vector<MapPoint> flags_;
    std::vector<Group> groups_;
    std::vector<Building*> ungrouped_;
    std::vector<Building*> buildings_;
    RoadNetworks roadNetworks_;

    rnet_id_t activePlanRoadNetwork_;
    bool activePlan_ = false;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDINGS_H_INCLUDED
