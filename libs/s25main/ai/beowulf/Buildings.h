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
#include "ai/beowulf/BuildingsBase.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/RoadIslands.h"
#include "ai/beowulf/BuildingQualityCalculator.h"

#include "ai/AIInterface.h"
#include "gameTypes/MapCoordinates.h"
#include "world/NodeMapBase.h"

#include <vector>
#include <utility> /* pair */

namespace beowulf {

/*
 * @todo: Why is BuildingsPlan not just a subclass of Buildings?
 *        Could those classes even merge to one?
 */
class Buildings : public BuildingsBase, public IBlockingReason
{
public:

    Buildings(AIInterface& aii);
    ~Buildings();

    // Construct
    void Construct(Building* building, const MapPoint& pos);
    void ConstructFlag(const MapPoint& pos);
    void ConstructRoad(const MapPoint& pos, const std::vector<Direction>& route);

    // Deconstruct
    void Deconstruct(Building* building);
    void DeconstructFlag(const MapPoint& pos);
    void DeconstructRoad(const MapPoint& pos, const std::vector<Direction>& route);

    // Islands
    RoadIslands& GetIslands() const;
    Island GetIsland(const MapPoint& pos) const override;
    MapPoint GetFlag(Island island) const;

    // Remove (remove the object from our data structure)
    void Remove(Building* building);
    void RemoveFlag(const MapPoint& pos);
    void RemoveRoad(const MapPoint& pos, const std::vector<Direction>& route);

    // Buildings
    Building* Create(BuildingType type, Building::State state, ProductionGroup group = InvalidProductionGroup, const MapPoint& pos = MapPoint::Invalid());
    const std::vector<Building*>& Get() const override;
    Building* Get(const MapPoint& pos) const override;
    bool HasBuilding(const MapPoint& pos) const override;
    void SetState(Building* building, Building::State state);
    void SetPos(Building* building, const MapPoint& pt);

    // Flags
    const std::vector<MapPoint>& GetFlags() const override;
    bool HasFlag(const MapPoint& pos) const override;
    FlagState GetFlagState(const MapPoint& pos) const;
    void SetFlagState(const MapPoint& pos, FlagState state);
    bool IsFlagConnected(const MapPoint& pos) const override;

    // Roads
    bool HasRoad(const MapPoint& pos, Direction dir) const override;
    RoadState GetRoadState(const MapPoint& pos, Direction dir) const;
    void SetRoadState(const MapPoint& pos, const std::vector<Direction>& route, RoadState state);
    //void AddRoadUser(Building* building, const std::vector<Direction>& route);

    BlockingManner GetBM(const MapPoint& pt) const override;
    std::pair<MapPoint, unsigned> GroupMemberDistance(const MapPoint& pt, ProductionGroup group, const std::vector<BuildingType>& types) const override;
    const GameWorldBase& GetWorld() const;

private:
    void SetRoadState(const MapPoint& pos, Direction dir, RoadState state);
    //void AddRoadUser(const MapPoint& pos, Direction dir, Building* building);
    //void RemoveRoadUser(Building* building);
    //bool HasRoadUser(const MapPoint& pos, Direction dir, Building* building);
    //void RemoveRoadUser(const MapPoint& pos, Direction dir, Building* building);
    bool InsertIntoExistingGroup(Building* building);

private:
    struct Node {
        Building* building          = nullptr;
        FlagState flag              = FlagDoesNotExist;
        RoadState roads[3]          { RoadDoesNotExist, RoadDoesNotExist, RoadDoesNotExist };
        //std::vector<Building*> users[3];
    };

    struct Group
    {
        std::vector<BuildingType> types;
        std::vector<Building*> buildings;
    };

    AIInterface& aii_;
    NodeMapBase<Node> nodes_;
    std::vector<MapPoint> flags_;
    std::vector<Group> groups_;
    std::vector<Building*> ungrouped_;
    std::vector<Building*> buildings_;
    RoadIslands islands_;
};

} // namespace beowulf

#endif //! BEOWULF_BUILDINGS_H_INCLUDED
