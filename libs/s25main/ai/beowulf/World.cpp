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

#include "ai/beowulf/World.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"
#include "nodeObjs/noFlag.h"
#include "pathfinding/PathConditionRoad.h"

#include <limits>

namespace beowulf {

World::World(AIInterface& aii)
    : aii_(aii), bqc_(*this, aii.gwb), roadNetworks_(*this)
{
    Resize(aii.gwb.GetSize());
    nodes_.Resize(GetSize());
    roadNetworks_.Resize(GetSize());

    // Set existing buildings.
    const nobHQ* bld = aii.GetHeadquarter();
    if (bld)
        SetPoint(Create(BLD_HEADQUARTERS, Building::Finished), bld->GetPos());

    for (const noBuildingSite* bld : aii.GetBuildingSites())
        SetPoint(Create(bld->GetBuildingType(), Building::UnderConstruction), bld->GetPos());

    for (const nobMilitary* bld : aii.GetMilitaryBuildings())
        SetPoint(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    for (unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (const nobUsual* bld : aii.GetBuildings(static_cast<BuildingType>(i)))
            SetPoint(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    // Set existing roads.
    RTTR_FOREACH_PT(MapPoint, GetSize()) {
        Node& node = nodes_[pt];
        if (aii.gwb.GetSpecObj<noFlag>(pt))
            SetFlagState(pt, FlagFinished);
        for (unsigned rdir = 0; rdir < 3; ++rdir) {
            if (aii.gwb.GetRoad(pt, rdir)) {
                if (aii.gwb.GetNO(pt)->GetType() != NOP_BUILDING &&
                        aii.gwb.GetNO(GetNeighbour(pt, OppositeDirection(Direction(rdir))))->GetType() != NOP_BUILDING)
                {
                    node.roads[rdir] = RoadFinished;
                }
            }
        }
    }

    roadNetworks_.Detect();
}

World::~World()
{
    for (Building* building : buildings_)
        delete building;
}

void World::Construct(Building* building, const MapPoint& pt)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::Finished);
    RTTR_Assert(building->GetState() != Building::ConstructionRequested);
    RTTR_Assert(building->GetState() != Building::UnderConstruction);

    ClearPlan();

    aii_.SetBuildingSite(pt, building->GetType());

    building->state_ = Building::ConstructionRequested;
    SetPoint(building, pt);
    if (!HasFlag(building->GetFlag()))
        SetFlagState(building->GetFlag(), FlagRequested);
}

void World::ConstructFlag(const MapPoint& pt)
{
    ClearPlan();

    Node& node = nodes_[pt];
    if (node.flag == FlagDoesNotExist) {
        aii_.SetFlag(pt);
        SetFlagState(pt, FlagRequested);
    }
}

void World::ConstructRoad(const MapPoint& pt, const std::vector<Direction>& route)
{
    ClearPlan();

    RTTR_Assert(nodes_[pt].flag == FlagRequested || nodes_[pt].flag == FlagFinished);
    aii_.BuildRoad(pt, false, route);
    SetRoadState(pt, route, RoadRequested);

    // @performance: if constructing multiple segments one final Detect() would suffice.
    roadNetworks_.Detect();
}

void World::Deconstruct(Building* building)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::PlanningRequest);

    ClearPlan();

    aii_.DestroyBuilding(building->GetPt());
    building->state_ = Building::DestructionRequested;
}

void World::DeconstructFlag(const MapPoint& pt)
{
    ClearPlan();

    Node& node = nodes_[pt];
    RTTR_Assert(node.flag == FlagFinished);

    const noFlag* nof = aii_.gwb.GetSpecObj<noFlag>(pt);
    RTTR_Assert(nof);
    aii_.DestroyFlag(nof);

    // Mark all connected roads as deconstruction requested as well.
    FloodFill(*this, pt,
    // condition
    [&](const MapPoint& pt2, Direction dir)
    {
        if (HasRoad(pt2, dir)) {
            SetRoadState(pt2, dir, RoadDestructionRequested);
            return !HasFlag(GetNeighbour(pt2, dir));
        } else
            return false;
    },
    // action
    [&](const MapPoint&) {});

    SetFlagState(pt, FlagDestructionRequested);
}

void World::DeconstructRoad(const MapPoint& pt, const std::vector<Direction>& route)
{
    RTTR_Assert(nodes_[pt].flag == FlagRequested || nodes_[pt].flag == FlagFinished);
    RTTR_Assert(!route.empty());
    RTTR_Assert(HasRoad(pt, route.front()));

    ClearPlan();

    aii_.DestroyRoad(pt, route.front());
    SetRoadState(pt, route, RoadDestructionRequested);

    roadNetworks_.Detect();
}

void World::Plan(Building* building, const MapPoint& pt)
{
    Node& node = nodes_[pt];
    RTTR_Assert(node.building == nullptr);
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() == Building::PlanningRequest);

    node.building = building;
    building->pt_ = pt;

    PlanFlag(GetNeighbour(pt, Direction::SOUTHEAST));

    activePlan_ = true;
}

void World::PlanFlag(const MapPoint& pt)
{
    Node& node = nodes_[pt];

    if (node.flag == FlagFinished || node.flag == FlagRequested)
        return;

    node.flagPlanCount++;

    if (node.flagPlanCount == 1) {
        flags_.push_back(pt);
        roadNetworks_.OnFlagStateChanged(pt, FlagRequested);
    }

    activePlan_ = true;
}

void World::PlanRoad(
        const MapPoint& pt,
        const std::vector<Direction>& route)
{
    MapPoint cur = pt;
    for (size_t i = 0; i < route.size(); ++i) {
        PlanSegment(cur, route[i]);
        cur = GetNeighbour(cur, route[i]);
    }

    PlanFlag(cur);
    roadNetworks_.Detect();

    activePlan_ = true;
}

void World::ClearPlan()
{
    if (!activePlan_)
        return;

    // Clear planned flags.
    flags_.erase(std::remove_if(flags_.begin(), flags_.end(),
        [&](const MapPoint& pt)
    {
        if (nodes_[pt].flagPlanCount > 0) {
            roadNetworks_.OnFlagStateChanged(pt, FlagDestructionRequested);
            return true;
        }
        return false;
    }), flags_.end());

    // Clear all nodes
    RTTR_FOREACH_PT(MapPoint, GetSize())
    {
        Node& node = nodes_[pt];

        if (node.building && node.building->GetState() == Building::PlanningRequest)
            node.building = nullptr;

        node.flagPlanCount = 0;
        node.roadPlanCount[0] = 0;
        node.roadPlanCount[1] = 0;
        node.roadPlanCount[2] = 0;
    }

    roadNetworks_.Detect();

    activePlan_ = false;
}

rnet_id_t World::GetRoadNetwork(const MapPoint& pt) const
{
    rnet_id_t ret = roadNetworks_.Get(pt);
    return ret;
}

MapPoint World::GetFlag(rnet_id_t rnet) const
{
    for (const MapPoint& flag : flags_)
        if (roadNetworks_.Get(flag) == rnet)
            return flag;
    return MapPoint::Invalid();
}

void World::Remove(Building* building)
{
    ClearPlan();

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

    if (building->GetPt().isValid())
        nodes_[building->GetPt()].building = nullptr;

    buildings_.erase(std::remove(buildings_.begin(), buildings_.end(), building), buildings_.end());

    delete building;
}

void World::RemoveFlag(const MapPoint& pt)
{
    ClearPlan();
    SetFlagState(pt, FlagDoesNotExist);
}

void World::RemoveRoad(
        const MapPoint& pt,
        const std::vector<Direction>& route)
{
    ClearPlan();
    SetRoadState(pt, route, RoadDoesNotExist);
    roadNetworks_.Detect();
}

Building* World::Create(
        BuildingType type,
        Building::State state,
        pgroup_id_t group,
        const MapPoint& pt)
{
    ClearPlan();

    Building* building = new Building(*this, type, state);
    building->pt_ = pt;
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

Building* World::Get(const MapPoint& pt) const
{
    if (pt.isValid())
        return nodes_[pt].building;
    return nullptr;
}

bool World::HasBuilding(const MapPoint& pt) const
{
    return Get(pt) != nullptr;
}

void World::SetState(Building* building, Building::State state)
{
    building->state_ = state;
}

bool World::HasFlag(const MapPoint& pt) const
{
    const Node& node = nodes_[pt];
    return node.flag == FlagRequested || node.flag == FlagFinished || node.flagPlanCount > 0;
}

FlagState World::GetFlagState(const MapPoint& pt) const
{
    return nodes_[pt].flag;
}

void World::SetFlagState(const MapPoint& pt, FlagState state)
{
    RTTR_Assert(nodes_[pt].flag != state);

    // new flag?
    if (!HasFlag(pt) && (state == FlagFinished || state == FlagRequested)) {
        flags_.push_back(pt);
    }

    // removed flag?
    if (HasFlag(pt) && (state == FlagDestructionRequested || state == FlagDoesNotExist)) {
        flags_.erase(std::find(flags_.begin(), flags_.end(), pt), flags_.end());
    }

    nodes_[pt].flag = state;
    roadNetworks_.OnFlagStateChanged(pt, state);
}

bool World::IsPointConnected(const MapPoint& pt) const
{
    for (unsigned dir = 0; dir < Direction::COUNT; ++dir) {
        if (HasRoad(pt, Direction(dir)))
            return true;
    }

    return false;
}

bool World::HasRoad(const MapPoint& pt, Direction dir) const
{
    MapPoint nodePt;
    unsigned idx;

    if (dir.toUInt() >= 3) {
        nodePt = pt;
        idx = OppositeDirection(dir).toUInt();
    } else {
        nodePt = GetNeighbour(pt, dir);
        idx = dir.toUInt();
    }

    const Node& node = nodes_[nodePt];
    RoadState state = node.roads[idx];
    return state == RoadRequested || state == RoadFinished || node.roadPlanCount[idx] > 0;
}

RoadState World::GetRoadState(const MapPoint& pt, Direction dir) const
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        return nodes_[pt].roads[oppositeDir.toUInt()];
    else
        return nodes_[GetNeighbour(pt, dir)].roads[dir.toUInt()];
}

void World::SetRoadState(const MapPoint& pt, Direction dir, RoadState state)
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        nodes_[pt].roads[oppositeDir.toUInt()] = state;
    else
        nodes_[GetNeighbour(pt, dir)].roads[dir.toUInt()] = state;
}

void World::SetRoadState(
        const MapPoint& pt,
        const std::vector<Direction>& route,
        RoadState state)
{
    MapPoint cur = pt;
    for (size_t i = 0; i < route.size(); ++i) {
        SetRoadState(cur, route[i], state);
        cur = GetNeighbour(cur, route[i]);
    }
}

BuildingQuality World::GetBQ(const MapPoint& pt) const
{
    BuildingQuality bq = bqc_(pt, [this, pt](const MapPoint& pt2){
        for (unsigned dir = 0; dir < Direction::COUNT; ++dir)
            if (HasRoad(pt2, Direction(dir)))
                return true;
        return false;
    });
    return aii_.gwb.AdjustBQ(pt, aii_.GetPlayerId(), bq);
}

bool World::IsRoadPossible(
        const MapPoint& pt,
        Direction dir) const
{
    MapPoint dest = GetNeighbour(pt, dir);

    if (HasFlag(dest))
        return true;

    for (unsigned d = 0; d < Direction::COUNT; ++d) {
        if (HasRoad(dest, Direction(d))) {
            return GetBQ(dest) >= BQ_FLAG;
        }
    }

    if (GetBM(dest) != BlockingManner::None)
        return false;

    return makePathConditionRoad(aii_.gwb, false).IsNodeOk(dest);
}

std::pair<MapPoint, unsigned> World::GetNearestBuilding(
        const MapPoint& pt,
        const std::vector<BuildingType>& types,
        rnet_id_t rnet) const
{
    // shortest euclidean distance.
    unsigned dist = std::numeric_limits<unsigned>::max();
    MapPoint closest = MapPoint::Invalid();

    uint64_t query = 0;
    for (BuildingType type : types)
        query |= 1ul << static_cast<uint64_t>(type);

    for (const Building* building : Get()) {
        // Skip buildings of different type.
        if (0 == (query & (1ul << static_cast<uint64_t>(building->GetType()))))
            continue;
        // Skip buildings that have no position yet.
        if (!building->GetPt().isValid())
            continue;
        // Skip buildings on different road networks.
        if (rnet != InvalidRoadNetwork && GetRoadNetwork(building->GetFlag()) != rnet)
            continue;

        unsigned d = CalcDistance(pt, building->GetPt());
        if (d < dist) {
            closest = building->GetPt();
            dist = d;
        }
    }

    return { closest, dist };
}

Building* World::GetGoodsDest(
        const Building* building,
        rnet_id_t rnet,
        const MapPoint& pt) const
{
    // Get destination infos for this type of building.
    bool checkGroup;
    const std::vector<BuildingType>& types = building->GetDestTypes(checkGroup);

    if (checkGroup) {
        for (Building* bld : Get()) {
            // Is the building in the same group?
            if (bld->GetGroup() != building->GetGroup())
                continue;

            // Does the building have the same type?
            if (std::find(types.begin(), types.end(), bld->GetType()) != types.end())
                return bld;
        }
    }

    auto nearest = GetNearestBuilding(pt, types, rnet);

    return nearest.first.isValid() ? Get(nearest.first) : nullptr;
}

bool World::InsertIntoExistingGroup(Building* building)
{
    for (pgroup_id_t gidx = 0; gidx < groups_.size(); ++gidx) {
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

void World::PlanSegment(const MapPoint& pt, Direction dir)
{
    if (dir.toUInt() >= 3)
        nodes_[pt].roadPlanCount[OppositeDirection(dir).toUInt()]++;
    else
        nodes_[GetNeighbour(pt, dir)].roadPlanCount[dir.toUInt()]++;
}

void World::SetPoint(Building* building, const MapPoint& pt)
{
    Node& node = nodes_[pt];
    RTTR_Assert(node.building == nullptr);

    if (building->pt_.isValid())
        nodes_[building->pt_].building = nullptr;

    node.building = building;
    building->pt_ = pt;
}

BlockingManner World::GetBM(const MapPoint& pt) const
{
    const Node& node = nodes_[pt];
    if (node.building)
        return BlockingManner::Building;

    if (HasFlag(pt))
        return BlockingManner::Flag;

    // Check for castle extensions
    for (unsigned dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
        Building* building = nodes_[GetNeighbour(pt, Direction(dir))].building;
        if (building && building->GetQuality() == BQ_CASTLE) {
            return BlockingManner::Single;
        }
    }

    return BlockingManner::None;
}

std::pair<MapPoint, unsigned>
World::GroupMemberDistance(
        const MapPoint& pt,
        pgroup_id_t group,
        const std::vector<BuildingType>& types) const
{
    std::pair<MapPoint, unsigned> ret{MapPoint::Invalid(), std::numeric_limits<unsigned>::max()};

    const Group& g = groups_[group];
    for (size_t i = 0; i < g.types.size(); ++i) {
        if (std::find(types.begin(), types.end(), g.types[i]) != types.end()) {
            if (g.buildings[i] == nullptr)
                continue;

            if (!g.buildings[i]->GetPt().isValid())
                continue;

            unsigned dist = CalcDistance(g.buildings[i]->GetFlag(), pt);
            if (dist < ret.second) {
                ret.second = dist;
                ret.first = g.buildings[i]->GetFlag();
            }
        }
    }

    return ret;
}

BlockingManner World::BQCalculator2::GetBM(const MapPoint& pt) const
{
    BlockingManner bm = world2.GetBM(pt);
    if (bm != BlockingManner::None)
        return bm;
    return world.GetNO(pt)->GetBM();
}

} // namespace beowulf
