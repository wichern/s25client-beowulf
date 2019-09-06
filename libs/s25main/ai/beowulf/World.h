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
#ifndef BEOWULF_WORLD_H_INCLUDED
#define BEOWULF_WORLD_H_INCLUDED

#include "ai/beowulf/Types.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/RoadNetwork.h"

#include "ai/AIInterface.h"
#include "gameTypes/MapCoordinates.h"
#include "world/NodeMapBase.h"
#include "world/BQCalculator.h"

#include <vector>
#include <utility> /* pair */

namespace beowulf {

/**
 * @brief Beowulfs world representation.
 *
 * This world representation supports planned buildings/flags/roads that have or have not
 * yet been sent to the game engine.
 *
 * What makes Buildings especially complex is that while the planner is running, the world can change.
 * Therefore, every change of the world (building/flag/roadsegment added or removed) will clear the plan.
 */
class World : public MapBase
{
public:
    World(AIInterface& aii);
    ~World();

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
    rnet_id_t GetRoadNetworkId(const MapPoint& pt) const { return nodes_[pt].roadNetworkId; }
    RoadNetwork* GetRoadNetwork(rnet_id_t rnet) const;

    // Remove (remove the object from our data structure)
    void Remove(Building* building);
    void RemoveFlag(const MapPoint& pt); // you probably want to use DeconstructFlag!
    void RemoveRoad(const MapPoint& pt, const std::vector<Direction>& route);

    // Buildings
    Building* Create(BuildingType type, Building::State state, pgroup_id_t group = InvalidProductionGroup, const MapPoint& pt = MapPoint::Invalid());
    const std::vector<Building*>& GetBuildings() const { return buildings_; }
    std::vector<Building*> GetBuildings(BuildingType type) const;
    Building* GetBuildings(const MapPoint& pt) const;
    bool HasBuilding(const MapPoint& pt) const;
    void SetState(Building* building, Building::State state);
    void SetPoint(Building* building, const MapPoint& pt);

    // Flags
    const std::vector<MapPoint>& GetFlags() const { return flags_; }
    bool HasFlag(const MapPoint& pt) const;
    FlagState GetFlagState(const MapPoint& pt) const;
    void SetFlagState(const MapPoint& pt, FlagState state);
    bool IsPointConnected(const MapPoint& pt) const;
    /// Get any flag connected to given road network. (Note: not updated while planning.)
    MapPoint GetFlag(rnet_id_t rnet) const;

    // Roads
    bool HasRoad(const MapPoint& pt, Direction dir) const;
    RoadState GetRoadState(const MapPoint& pt, Direction dir) const;
    void SetRoadState(const MapPoint& pt, const std::vector<Direction>& route, RoadState state);

    BuildingQuality GetBQ(const MapPoint& pt) const;

    /// Get building of type 'types' in group 'group' with the shortest flag distance.
    std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, pgroup_id_t group, const std::vector<BuildingType>& types) const;

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
    void SetRoadState(const MapPoint& pt, Direction dir, RoadState state);
    bool InsertIntoExistingGroup(Building* building);
    void PlanSegment(const MapPoint& pt, Direction dir);
    BlockingManner GetBM(const MapPoint& pt) const;

private:
    AIInterface& aii_;

    /*
     * Additional Beowulf data on every map point.
     */
    struct Node {
        Building* building          = nullptr;
        FlagState flag              = FlagDoesNotExist;
        unsigned flagPlanCount      = 0;
        RoadState roads[3]          = { RoadDoesNotExist, RoadDoesNotExist, RoadDoesNotExist };
        unsigned roadPlanCount[3]   = { 0, 0, 0 };
        rnet_id_t roadNetworkId     = InvalidRoadNetwork;
    };
    NodeMapBase<Node> nodes_;

    /*
     * Buildings producing goods are combined in groups.
     */
    struct Group
    {
        std::vector<BuildingType> types;
        std::vector<Building*> buildings;
    };
    std::vector<Group> groups_;

    class BQCalculator2 : public BQCalculator
    {
    public:
        BQCalculator2(World& w2, const GameWorldBase& gw) : BQCalculator(gw), world2(w2) {}
    private:
        BlockingManner GetBM(const MapPoint& pt) const override;
        World& world2;
    } bqc_;

    std::vector<MapPoint> flags_;
    std::vector<Building*> ungrouped_;
    std::vector<Building*> buildings_;

    bool activePlan_ = false;

    /*
     * Road networks
     */
    std::vector<RoadNetwork*> roadNetworks_;
    rnet_id_t nextRoadNetworkId_ = 0;
    void RoadNetworkOnFlagAdded(const MapPoint& pt);
    void RoadNetworkOnFlagRemoved(const MapPoint& pt);
    void RoadNetworkOnRoadAdded(const MapPoint& start, const std::vector<Direction>& route);
    void RoadNetworkOnRoadRemoved(const MapPoint& start, const std::vector<Direction>& route);
    void DetectRoadNetworks();
    void MergeRoadNetworks(rnet_id_t a, rnet_id_t b);
};

} // namespace beowulf

#endif //! BEOWULF_WORLD_H_INCLUDED
