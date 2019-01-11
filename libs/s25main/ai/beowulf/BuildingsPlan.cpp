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

#include "ai/beowulf/BuildingsPlan.h"
#include "ai/beowulf/Helper.h"

namespace beowulf {

BuildingsPlan::BuildingsPlan(const Buildings& buildings, Island island)
    : island_(island),
      buildings_(buildings),
      buildingsVec_(buildings.Get()),
      firstPlanned_(buildings.Get().size()),
      flags_(buildings.GetFlags())
{
    nodes_.Resize(buildings.GetWorld().GetSize());
}

void BuildingsPlan::PlanBuilding(const MapPoint& pos, Building* bld)
{
    Node& node = nodes_[pos];
    RTTR_Assert(node.building == nullptr);
    RTTR_Assert(bld);
    node.building = bld;
    bld->pos_ = pos;
    buildingsVec_.push_back(bld);
    PlanFlag(nodes_.GetNeighbour(pos, Direction::SOUTHEAST));
}

void BuildingsPlan::UnplanBuilding(const MapPoint& pos)
{
    RTTR_Assert(nodes_[pos].building);
    nodes_[pos].building->pos_ = MapPoint::Invalid();
    nodes_[pos].building = nullptr;
    buildingsVec_.erase(std::find(buildingsVec_.begin() + firstPlanned_, buildingsVec_.end(), nodes_[pos].building), buildingsVec_.end());
    UnplanFlag(nodes_.GetNeighbour(pos, Direction::SOUTHEAST));
}

void BuildingsPlan::PlanFlag(const MapPoint& pos)
{
    Node& node = nodes_[pos];

    node.flag++;

    if (node.flag == 1)
        flags_.push_back(pos);
}

void BuildingsPlan::UnplanFlag(const MapPoint& pos)
{
    Node& node = nodes_[pos];

    RTTR_Assert(node.flag > 0);

    node.flag--;

    if (node.flag == 0)
        flags_.erase(std::remove(flags_.begin(), flags_.end(), pos));
}

void BuildingsPlan::PlanRoad(const MapPoint& pos, const std::vector<Direction>& route)
{
    MapPoint cur = pos;
    for (size_t i = 0; i < route.size(); ++i) {
        PlanSegment(cur, route[i]);

        if (i > 1 && i < route.size() - 2)
            if (GetBM(cur) == BlockingManner::None)
                possibleFlags_.push_back(cur);

        cur = nodes_.GetNeighbour(cur, route[i]);
    }

    PlanFlag(cur);
}

void BuildingsPlan::UnplanRoad(const MapPoint& pos, const std::vector<Direction>& route)
{
    MapPoint cur = pos;
    for (size_t i = 0; i < route.size(); ++i) {
        UnplanSegment(cur, route[i]);

        if (i > 1 && i < route.size() - 2)
            if (GetBM(cur) == BlockingManner::None)
                possibleFlags_.erase(std::remove(possibleFlags_.begin(), possibleFlags_.end(), cur), possibleFlags_.end());

        cur = nodes_.GetNeighbour(cur, route[i]);
    }

    UnplanFlag(cur);
}

void BuildingsPlan::PlanSegment(const MapPoint& pos, Direction dir)
{
    if (dir.toUInt() >= 3)
        nodes_[pos].road[OppositeDirection(dir).toUInt()]++;
    else
        nodes_[nodes_.GetNeighbour(pos, dir)].road[dir.toUInt()]++;
}

void BuildingsPlan::UnplanSegment(const MapPoint& pos, Direction dir)
{
    if (dir.toUInt() >= 3)
        nodes_[pos].road[OppositeDirection(dir).toUInt()]--;
    else
        nodes_[nodes_.GetNeighbour(pos, dir)].road[dir.toUInt()]--;
}

void BuildingsPlan::Clear()
{
    RTTR_FOREACH_PT(MapPoint, nodes_.GetSize()) {
        Node& node = nodes_[pt];
        memset(&node, 0, sizeof(node));
        int check = 10;
        (void)check;
    }
}

bool BuildingsPlan::IsRoadPossible(
        const GameWorldBase& gwb,
        const MapPoint& pos,
        Direction dir) const
{
    // @todo: This is not how we can check whether it is possible to place a flag.
    if (IsFlagConnected(pos) && GetBM(pos) == BlockingManner::None)
        return true;

    MapPoint dest = gwb.GetNeighbour(pos, dir);

    if (IsFlagConnected(dest) && GetBM(dest) == BlockingManner::None)
        return true;

    return BuildingsBase::IsRoadPossible(gwb, pos, dir);
}

std::vector<MapPoint> BuildingsPlan::GetPossibleJoints(const MapPoint& pos, Island island) const
{
    std::vector<MapPoint> ret(flags_);
    ret.insert(ret.end(), possibleFlags_.begin(), possibleFlags_.end());
    ret.erase(std::remove(ret.begin(), ret.end(), pos), ret.end());
    for (const MapPoint& flag : buildings_.GetFlags())
        if (buildings_.GetIsland(flag) == island)
            ret.push_back(flag);
    return ret;
}

Island BuildingsPlan::GetIsland(const MapPoint& pos) const
{
    Island ret = buildings_.GetIsland(pos);
    if (ret == InvalidIsland)
        if (nodes_[pos].flag > 0)
            ret = island_;
    return ret;
}

const std::vector<Building*>& BuildingsPlan::Get() const
{
    return buildingsVec_;
}

Building* BuildingsPlan::Get(const MapPoint& pos) const
{
    Building* ret = buildings_.Get(pos);
    if (ret == nullptr)
        return nodes_[pos].building;
    return ret;
}

const std::vector<MapPoint>& BuildingsPlan::GetFlags() const
{
    return flags_;
}

bool BuildingsPlan::HasFlag(const MapPoint& pos) const
{
    if (buildings_.HasFlag(pos))
        return true;
    return nodes_[pos].flag > 0;
}

bool BuildingsPlan::HasRoad(const MapPoint& pos, Direction dir) const
{
    if (buildings_.HasRoad(pos, dir))
        return true;

    if (dir.toUInt() >= 3)
        return nodes_[pos].road[OppositeDirection(dir).toUInt()] > 0;
    else
        return nodes_[nodes_.GetNeighbour(pos, dir)].road[dir.toUInt()] > 0;
}

bool BuildingsPlan::HasBuilding(const MapPoint& pos) const
{
    if (buildings_.HasBuilding(pos))
        return true;

    return nodes_[pos].building != nullptr;
}

bool BuildingsPlan::IsFlagConnected(const MapPoint& pos) const
{
    for (unsigned dir = 0; dir < Direction::COUNT; ++dir)
        if (HasRoad(pos, Direction(dir)))
            return true;

    return false;
}

BlockingManner BuildingsPlan::GetBM(const MapPoint& pt) const
{
    const Node& node = nodes_[pt];
    if (node.building || buildings_.Get(pt))
        return BlockingManner::Building;

    if (HasFlag(pt))
        return BlockingManner::Flag;

    // flagsAround probably has a different meaning :/
//    for (unsigned dir = 0; dir < Direction::COUNT; ++dir)
//        if (HasFlag(nodes_.GetNeighbour(pt, Direction(dir))))
//            return BlockingManner::FlagsAround;

    return BlockingManner::None;
}

bool BuildingsPlan::IsOnRoad(const MapPoint& pt) const
{
    for (unsigned dir = 0; dir < Direction::COUNT; ++dir)
        if (HasRoad(pt, Direction(dir)))
            return true;
    return false;
}

std::pair<MapPoint, unsigned> BuildingsPlan::GroupMemberDistance(
        const MapPoint& pt,
        ProductionGroup group,
        const std::vector<BuildingType>& types) const
{
    return buildings_.GroupMemberDistance(pt, group, types);
}

} // namespace beowulf
