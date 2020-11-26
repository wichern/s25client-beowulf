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
#include "ai/beowulf/Resources.h"

#include "ai/AIInterface.h"
#include "gameTypes/MapCoordinates.h"
#include "world/NodeMapBase.h"
#include "world/BQCalculator.h"
#include "notifications/Subscription.h"

#include <vector>
#include <utility> /* pair */

class BuildingNote;
class RoadNote;
class FlagNote;

namespace beowulf {

class Beowulf;

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
    World(Beowulf* beowulf, bool fow);
    virtual ~World();

    Resources resources;

    struct ProductionGroup
    {
        const std::vector<BuildingType> types;
        std::vector<Building*> buildings;
        const unsigned id;
        const MapPoint region;
    };
    std::vector<ProductionGroup> groups;

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

    // Remove (remove the object from our data structure)
    void Remove(Building* building);
    void RemoveFlag(const MapPoint& pt); // you probably want to use DeconstructFlag!
    void RemoveRoad(const MapPoint& pt, const std::vector<Direction>& route);

    // Buildings
    Building* Create(BuildingType type, Building::State state, unsigned group = InvalidProductionGroup, const MapPoint& pt = MapPoint::Invalid());
    const std::vector<Building*>& GetBuildings() const { return buildings_; }
    std::vector<Building*> GetBuildings(BuildingType type) const;
    Building* GetBuilding(const MapPoint& pt) const;
    Building* GetHQ() const;
    const MapPoint& GetHQFlag() const { return hqFlag_; }
    bool HasBuilding(const MapPoint& pt) const;
    void SetState(Building* building, Building::State state);
    void SetPoint(Building* building, const MapPoint& pt);
    void SetCaptured(Building* building);
    Building* GetGoodsDest(const Building* building, const MapPoint& pt) const;
    const noBaseBuilding* GetBaseBuilding(const Building* building) const;
    ProductionGroup& CreateGroup(const std::vector<BuildingType>&& types, const MapPoint& region);

    // Flags
    const std::vector<MapPoint>& GetFlags() const { return flags_; }
    bool HasFlag(const MapPoint& pt) const;
    FlagState GetFlagState(const MapPoint& pt) const;
    void SetFlagState(const MapPoint& pt, FlagState state);
    bool IsPointConnected(const MapPoint& pt) const;

    // Roads
    bool HasRoad(const MapPoint& pt, Direction dir) const;
    bool IsOnRoad(const MapPoint& pt) const;
    RoadState GetRoadState(const MapPoint& pt, Direction dir) const;
    void SetRoadState(const MapPoint& pt, const std::vector<Direction>& route, RoadState state);
    /// Check if we can build a segment from 'pt' in 'dir'. Assumes that we can start at 'pt'.
    bool IsRoadPossible(const MapPoint& pt, Direction dir, bool includeAnticipated, const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps = {}) const;
    std::vector<Direction> GetPath(const MapPoint& src, const MapPoint& dst) const;
    bool CanConnectBuilding(const MapPoint& buildingFlag, const MapPoint& dst, bool includeAnticipated, const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps = {}) const;
    bool IsConnected(const MapPoint& src, const MapPoint& dst) const;

    // Border/Territory
    bool IsBorder(const MapPoint& pt, bool includeAnticipated) const;
    bool IsPlayerTerritory(const MapPoint& pt, bool includeAnticipated) const;
    /// Check if the given point is part of player territory or will become
    /// part of it once a military building finishes.
    bool IsOwner(const MapPoint& pt, bool includeAnticipated) const;

    // Building Quality
    BuildingQuality GetBQ(const MapPoint& pt, bool includeAnticipated, const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps = {}) const;

    // Helper
    std::vector<const noBaseBuilding*> GetEnemyCatapultsInReach(const MapPoint& pt) const;
    /// Get the number of enemy soldiers that can attack us here.
    /// Based on the number of military buildings we can see.
    unsigned GetEnemySoldiersInReach(const MapPoint& pt) const;
    /// Check whether we can see enemy territory in radius.
    bool IsEnemyNear(const MapPoint& pt, unsigned radius) const;

    /// Get building of type 'types' in group 'group' with the shortest flag distance.
    std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, unsigned group, BuildingType type) const;
    unsigned GetMaxGroupMemberDistance(const MapPoint& pt, unsigned group, BuildingType type) const;
    std::pair<MapPoint, unsigned> GetNearestBuilding(const MapPoint& pt, const std::vector<BuildingType>& types, const Building* self = nullptr) const;
    bool CanBuildMilitary(const MapPoint& pt) const;

    /// Predict the results of a territory expansion.
    /// Can be used for capturing enemy buildings as well.
    void PredictExpansionResults(
            const MapPoint& pt, BuildingType type,
            std::vector<MapPoint>& additionalTerritory,
            std::vector<const noBaseBuilding*>& destroyed) const;

    bool IsPlanning() const { return activePlan_; }

    bool IsRestricted(const MapPoint& pt) const;
    bool IsVisible(const MapPoint& pt) const;

private:
    void SetRoadState(const MapPoint& pt, Direction dir, RoadState state);
    void PlanSegment(const MapPoint& pt, Direction dir);
    BlockingManner GetBM(const MapPoint& pt, const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps = {}) const;
    BuildingQuality AdjustBQ(const MapPoint pt, BuildingQuality nodeBQ, bool includeAnticipated) const;

    void OnBuildingNote(const BuildingNote& note);
    void OnRoadNote(const RoadNote& note);
    void OnFlagNote(const FlagNote& note);

    bool IsCloserThanEnemy(const MapPoint& pt, unsigned distance, std::vector<const nobBaseMilitary*> enemies) const;

    const noBaseBuilding* GetEnemyBuilding(const MapPoint& pt) const;

private:
    Beowulf* beowulf_;
    AIInterface& aii_;
    bool fow_;
    std::vector<Subscription> eventSubscriptions_;

    /*
     * Additional Beowulf data on every map point.
     */
    struct Node {
        Building* building          = nullptr;
        FlagState flag              = FlagDoesNotExist;
        unsigned flagPlanCount      = 0;
        RoadState roads[3]          = { RoadDoesNotExist, RoadDoesNotExist, RoadDoesNotExist };
        unsigned roadPlanCount[3]   = { 0, 0, 0 };
    };
    NodeMapBase<Node> nodes_;

    /**
     * Adding a layer of planned buildings to the BQCalculator.
     */
    class BQCalculator2 : public BQCalculator
    {
    public:
        BQCalculator2(const World& w2, const GameWorldBase& gw, const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps) :
            BQCalculator(gw), world2(w2), tmps_(tmps) {}
        virtual ~BQCalculator2() = default;

    private:
        BlockingManner GetBM(const MapPoint& pt) const override;
        const World& world2;

        // Theoretical buildings to be considered. (used for CanConnect())
        const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps_;
    };

    std::vector<MapPoint> flags_;
    std::vector<Building*> buildings_;

    bool activePlan_ = false;
    MapPoint hqFlag_;
};

} // namespace beowulf

#endif //! BEOWULF_WORLD_H_INCLUDED
