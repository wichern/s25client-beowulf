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

#include "rttrDefines.h" // IWYU pragma: keep

#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"
#include "nodeObjs/noFlag.h"

#include <limits>

namespace beowulf {

Buildings::Buildings(AIInterface& aii)
    : aii_(aii), islands_(aii.gwb)
{
    nodes_.Resize(aii.gwb.GetSize());

    // Set existing buildings.
    const nobHQ* bld = aii.GetHeadquarter();
    if (bld)
        SetPos(Create(BLD_HEADQUARTERS, Building::Finished), bld->GetPos());

    for (const noBuildingSite* bld : aii.GetBuildingSites())
        SetPos(Create(bld->GetBuildingType(), Building::UnderConstruction), bld->GetPos());

    for (const nobMilitary* bld : aii.GetMilitaryBuildings())
        SetPos(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    for (unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (const nobUsual* bld : aii.GetBuildings(static_cast<BuildingType>(i)))
            SetPos(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    // Set existing roads.
    RTTR_FOREACH_PT(MapPoint, aii.gwb.GetSize()) {
        Node& node = nodes_[pt];
        if (aii.gwb.GetSpecObj<noFlag>(pt))
            SetFlagState(pt, FlagFinished);
        for (unsigned rdir = 0; rdir < 3; ++rdir) {
            if (aii.gwb.GetRoad(pt, rdir)) {
                if (aii.gwb.GetNO(pt)->GetType() != NOP_BUILDING &&
                        aii.gwb.GetNO(aii.gwb.GetNeighbour(pt, OppositeDirection(Direction(rdir))))->GetType() != NOP_BUILDING)
                {
                    node.roads[rdir] = RoadFinished;
                }
            }
        }
    }

    islands_.Detect(this);
}

Buildings::~Buildings()
{
    for (Building* building : buildings_)
        delete building;
}

void Buildings::Construct(Building* building, const MapPoint& pos)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::Finished);
    RTTR_Assert(building->GetState() != Building::ConstructionRequested);
    RTTR_Assert(building->GetState() != Building::UnderConstruction);

    aii_.SetBuildingSite(pos, building->GetType());

    building->state_ = Building::ConstructionRequested;
    SetPos(building, pos);
    if (!HasFlag(building->GetFlag()))
        SetFlagState(building->GetFlag(), FlagRequested);
}

void Buildings::ConstructFlag(const MapPoint& pos)
{
    Node& node = nodes_[pos];
    if (node.flag == FlagDoesNotExist) {
        aii_.SetFlag(pos);
        SetFlagState(pos, FlagRequested);
    }
}

void Buildings::ConstructRoad(const MapPoint& pos, const std::vector<Direction>& route)
{
    RTTR_Assert(nodes_[pos].flag == FlagRequested || nodes_[pos].flag == FlagFinished);
    aii_.BuildRoad(pos, false, route);
    SetRoadState(pos, route, RoadRequested);

    // @todo: if constructing multiple segments one final DetectIslands() would suffice.
    islands_.Detect(this);
}

void Buildings::Deconstruct(Building* building)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::PlanningRequest);

    aii_.DestroyBuilding(building->GetPos());
    building->state_ = Building::DestructionRequested;
}

void Buildings::DeconstructFlag(const MapPoint& pos)
{
    Node& node = nodes_[pos];
    RTTR_Assert(node.flag == FlagFinished);

    const noFlag* nof = aii_.gwb.GetSpecObj<noFlag>(pos);
    RTTR_Assert(nof);
    aii_.DestroyFlag(nof);

    // Mark all connected roads as deconstruction requested as well.
    FloodFill(aii_.gwb, pos,
    // condition
    [&](const MapPoint& pos, Direction dir)
    {
        if (HasRoad(pos, dir)) {
            SetRoadState(pos, dir, RoadDestructionRequested);
            return !HasFlag(aii_.gwb.GetNeighbour(pos, dir));
        } else {
            return false;
        }
    },
    // action
    [&](const MapPoint& pos)
    {
        (void)pos;
    });

    SetFlagState(pos, FlagDestructionRequested);
}

void Buildings::DeconstructRoad(const MapPoint& pos, const std::vector<Direction>& route)
{
    RTTR_Assert(nodes_[pos].flag == FlagRequested || nodes_[pos].flag == FlagFinished);
    RTTR_Assert(!route.empty());
    RTTR_Assert(HasRoad(pos, route.front()));

    aii_.DestroyRoad(pos, route.front());
    SetRoadState(pos, route, RoadDestructionRequested);

    islands_.Detect(this);
}

Island Buildings::GetIsland(const MapPoint& pos) const
{
    return islands_.Get(pos);
}

MapPoint Buildings::GetFlag(Island island) const
{
    for (const MapPoint& flag : flags_)
        if (islands_.Get(flag) == island)
            return flag;
    return MapPoint::Invalid();
}

void Buildings::Remove(Building* building)
{
    if (building->GetGroup() != InvalidProductionGroup) {
        Group& group = groups_[building->GetGroup()];
        size_t nullEntries = 0;
        for (size_t i = 0; i < group.types.size(); ++i) {
            if (group.buildings[i] == building)
                group.buildings[i] = nullptr;

            if (group.buildings[i] == nullptr)
                nullEntries++;
        }

        if (nullEntries == group.buildings.size())
            groups_.erase(groups_.begin() + building->GetGroup());
    }

    if (building->GetPos().isValid())
        nodes_[building->GetPos()].building = nullptr;

    buildings_.erase(std::remove(buildings_.begin(), buildings_.end(), building), buildings_.end());
    //RemoveRoadUser(building);

    delete building;
}

void Buildings::RemoveFlag(const MapPoint& pos)
{
    SetFlagState(pos, FlagDoesNotExist);
}

void Buildings::RemoveRoad(const MapPoint& pos, const std::vector<Direction>& route)
{
    SetRoadState(pos, route, RoadDoesNotExist);
    islands_.Detect(this);
}

Building* Buildings::Create(
        BuildingType type,
        Building::State state,
        ProductionGroup group,
        const MapPoint& pos)
{
    Building* building = new Building(*this, type, state);
    building->pos_ = pos;
    buildings_.push_back(building);

    if (group != InvalidProductionGroup) {
        building->group_ = group;
        return building;
    }

    if (building->IsProduction()) {
        // Find group with free spot
        if (InsertIntoExistingGroup(building))
            return building;

        // Create a new group

        Group group;
        switch (type)
        {
        case BLD_WOODCUTTER:
        case BLD_FORESTER:
        case BLD_SAWMILL:
        {
            group.types = { BLD_SAWMILL, BLD_WOODCUTTER, BLD_WOODCUTTER, BLD_FORESTER };
        } break;
        case BLD_SLAUGHTERHOUSE:
        case BLD_PIGFARM:
        {
            group.types = { BLD_SLAUGHTERHOUSE, BLD_PIGFARM, BLD_FARM, BLD_FARM, BLD_WELL };
        } break;
        case BLD_BREWERY:
        {
            group.types = { BLD_BREWERY, BLD_FARM, BLD_WELL };
        } break;
        case BLD_ARMORY:
        {
            group.types = { BLD_ARMORY, BLD_IRONSMELTER };
        } break;
        case BLD_METALWORKS:
        {
            group.types = { BLD_METALWORKS, BLD_IRONSMELTER };
        } break;
        case BLD_MILL:
        case BLD_BAKERY:
        {
            group.types = { BLD_BAKERY, BLD_MILL, BLD_FARM, BLD_FARM, BLD_WELL };
        } break;
        case BLD_DONKEYBREEDER:
        {
            group.types = { BLD_DONKEYBREEDER, BLD_FARM, BLD_FARM, BLD_WELL };
        } break;
        default:
        {
            // This type of building cannot start a new group.
            ungrouped_.push_back(building);
            return building;
        }
        }

        group.buildings.resize(group.types.size());

        for (size_t i = 0; i < group.types.size(); ++i) {
            if (group.types[i] == type) {
                group.buildings[i] = building;
                break;
            }
        }

        building->group_ = groups_.size();
        groups_.push_back(group);

        // Since a new group was created, we can try to match ungrouped buildings.
        ungrouped_.erase(std::remove_if(ungrouped_.begin(), ungrouped_.end(),
            [&](Building* b) { return InsertIntoExistingGroup(b); }), ungrouped_.end());
    }

    return building;
}

const std::vector<Building*>& Buildings::Get() const
{
    return buildings_;
}

Building* Buildings::Get(const MapPoint& pos) const
{
    if (pos.isValid())
        return nodes_[pos].building;
    return nullptr;
}

bool Buildings::HasBuilding(const MapPoint& pos) const
{
    return Get(pos) != nullptr;
}

void Buildings::SetState(Building* building, Building::State state)
{
    building->state_ = state;
}

const std::vector<MapPoint>& Buildings::GetFlags() const
{
    return flags_;
}

bool Buildings::HasFlag(const MapPoint& pos) const
{
    FlagState state = GetFlagState(pos);
    return state == FlagRequested || state == FlagFinished;
}

FlagState Buildings::GetFlagState(const MapPoint& pos) const
{
    return nodes_[pos].flag;
}

void Buildings::SetFlagState(const MapPoint& pos, FlagState state)
{
    RTTR_Assert(nodes_[pos].flag != state);

    // new flag?
    if (!HasFlag(pos) && (state == FlagFinished || state == FlagRequested)) {
        flags_.push_back(pos);
    }

    // removed flag?
    if (HasFlag(pos) && (state == FlagDestructionRequested || state == FlagDoesNotExist)) {
        flags_.erase(std::find(flags_.begin(), flags_.end(), pos), flags_.end());
    }

    nodes_[pos].flag = state;
    islands_.OnFlagStateChanged(this, pos, state);
}

bool Buildings::IsFlagConnected(const MapPoint& pos) const
{
    RTTR_Assert(HasFlag(pos));

    for (unsigned dir = 0; dir < Direction::COUNT; ++dir) {
        if (HasRoad(pos, Direction(dir)))
            return true;
    }

    return false;
}

bool Buildings::HasRoad(const MapPoint& pos, Direction dir) const
{
    RoadState state = GetRoadState(pos, dir);
    return state == RoadRequested || state == RoadFinished;
}

RoadState Buildings::GetRoadState(const MapPoint& pos, Direction dir) const
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        return nodes_[pos].roads[oppositeDir.toUInt()];
    else
        return nodes_[nodes_.GetNeighbour(pos, dir)].roads[dir.toUInt()];
}

void Buildings::SetRoadState(const MapPoint& pos, Direction dir, RoadState state)
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        nodes_[pos].roads[oppositeDir.toUInt()] = state;
    else
        nodes_[nodes_.GetNeighbour(pos, dir)].roads[dir.toUInt()] = state;
}

void Buildings::SetRoadState(const MapPoint& pos, const std::vector<Direction>& route, RoadState state)
{
    MapPoint cur = pos;
    for (size_t i = 0; i < route.size(); ++i) {
        SetRoadState(cur, route[i], state);
        cur = nodes_.GetNeighbour(cur, route[i]);
    }
}

//void Buildings::AddRoadUser(Building* building, const std::vector<Direction>& route)
//{
//    MapPoint cur = building->GetFlag();
//    for (unsigned i = 0; i < route.size(); ++i) {
//        AddRoadUser(cur, route[i], building);
//        cur = nodes_.GetNeighbour(cur, route[i]);
//    }
//}

//void Buildings::RemoveRoadUser(Building* building)
//{
//    FloodFill(
//    // World
//    GetWorld(),
//    // Start
//    building->GetFlag(),
//    // Condition
//    [&](const MapPoint& pt, Direction dir)
//    {
//        if (HasRoadUser(pt, dir, building)) {
//            RemoveRoadUser(pt, dir, building);
//            return true;
//        }
//        return false;
//    },
//    // Action
//    [&](const MapPoint& pt) { (void)(pt); });
//}

const GameWorldBase& Buildings::GetWorld() const
{
    return aii_.gwb;
}

//void Buildings::AddRoadUser(const MapPoint& pos, Direction dir, Building* building)
//{
//    if (dir.toUInt() < 3) {
//        AddRoadUser(aii_.gwb.GetNeighbour(pos, dir), OppositeDirection(dir), building);
//        return;
//    }

//    std::vector<Building*>& users = nodes_[pos].users[dir.toUInt() - 3];
//    if (std::find(users.begin(), users.end(), building) != users.end())
//        users.push_back(building);
//}

//bool Buildings::HasRoadUser(const MapPoint& pos, Direction dir, Building* building)
//{
//    if (dir.toUInt() < 3)
//        return HasRoadUser(aii_.gwb.GetNeighbour(pos, dir), OppositeDirection(dir), building);

//    const std::vector<Building*>& users = nodes_[pos].users[dir.toUInt() - 3];
//    return std::find(users.begin(), users.end(), building) != users.end();
//}

//void Buildings::RemoveRoadUser(const MapPoint& pos, Direction dir, Building* building)
//{
//    if (dir.toUInt() < 3) {
//        AddRoadUser(aii_.gwb.GetNeighbour(pos, dir), OppositeDirection(dir), building);
//        return;
//    }

//    std::vector<Building*>& users = nodes_[pos].users[dir.toUInt() - 3];
//    users.erase(std::remove(users.begin(), users.end(), building), users.end());
//}

bool Buildings::InsertIntoExistingGroup(Building* building)
{
    for (ProductionGroup gidx = 0; gidx < groups_.size(); ++gidx) {
        Group& group = groups_[gidx];
        for (size_t i = 0; i < group.types.size(); ++i) {
            if (group.buildings[i] == nullptr && group.types[i] == building->GetType()) {
                group.buildings[i] = building;
                building->group_ = gidx;
                return true;
            }
        }
    }

    return false;
}

void Buildings::SetPos(Building* building, const MapPoint& pt)
{
    Node& node = nodes_[pt];
    RTTR_Assert(node.building == nullptr);

    if (building->pos_.isValid())
        nodes_[building->pos_].building = nullptr;

    node.building = building;
    building->pos_ = pt;
}

BlockingManner Buildings::GetBM(const MapPoint& pt) const
{
    const Node& node = nodes_[pt];
    if (node.building)
        return BlockingManner::Building;

    if (HasFlag(pt))
        return BlockingManner::Flag;

    // @hint: flags around probably has a different meaning :/
//    for (unsigned dir = 0; dir < Direction::COUNT; ++dir) {
//        if (HasFlag(nodes_.GetNeighbour(pt, Direction(dir))))
//            return BlockingManner::FlagsAround;
//    }

    return BlockingManner::None;
}

std::pair<MapPoint, unsigned>
Buildings::GroupMemberDistance(
        const MapPoint& pt,
        ProductionGroup group,
        const std::vector<BuildingType>& types) const
{
    std::pair<MapPoint, unsigned> ret{MapPoint::Invalid(), std::numeric_limits<unsigned>::max()};

    const Group& g = groups_[group];
    for (size_t i = 0; i < g.types.size(); ++i) {
        if (std::find(types.begin(), types.end(), g.types[i]) != types.end()) {
            if (g.buildings[i] == nullptr)
                continue;

            if (!g.buildings[i]->GetPos().isValid())
                continue;

            unsigned dist = aii_.gwb.CalcDistance(g.buildings[i]->GetFlag(), pt);
            if (dist < ret.second) {
                ret.second = dist;
                ret.first = g.buildings[i]->GetFlag();
            }
        }
    }

    return ret;
}

} // namespace beowulf
