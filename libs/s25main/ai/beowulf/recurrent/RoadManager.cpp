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

#include "ai/beowulf/recurrent/RoadManager.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/ProductionConsts.h"
#include "ai/beowulf/BuildLocations.h"
#include "ai/beowulf/Debug.h"

#include "notifications/BuildingNote.h"
#include "notifications/RoadNote.h"
#include "helpers/containerUtils.h"

namespace beowulf {

RoadManager::RoadManager(Beowulf* beowulf)
    : RecurrentBase(beowulf)
{
    nodes_.Resize(beowulf->world.GetSize());
}

void RoadManager::OnRun()
{
    /*
     * @todo: Try to connect a different building every time.
     */
}

bool RoadManager::Connect(const Building* building, BuildLocations* buildLocations)
{
    // Get building we want to connect to.
    Building* destBuilding = beowulf_->world.GetGoodsDest(building, building->GetPt());
    if (!(destBuilding && destBuilding->GetPt().isValid())) {
        // At least connect it to a store house.
        auto nearest = beowulf_->world.GetNearestBuilding(
                    building->GetPt(),
                    { BLD_HEADQUARTERS, BLD_STOREHOUSE, BLD_HARBORBUILDING },
                    building);
        destBuilding = beowulf_->world.GetBuilding(nearest.first);
        if (!destBuilding)
            return false;
    }

    // Find a suitable path.
    std::vector<Direction> route;
    MapPoint start = building->GetFlag();
    MapPoint dest = destBuilding->GetFlag();
    bool ret = FindPath(start, beowulf_->world, &route,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        if (pt == start && dir == Direction::NORTHWEST)
            return false;
        return beowulf_->world.HasRoad(pt, dir) || beowulf_->world.IsRoadPossible(pt, dir, false);
    },
    // End
    [&](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == dest;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return beowulf_->world.CalcDistance(pt, dest);
    },
    // Cost
    [&](const MapPoint& pt, Direction dir)
    {
        unsigned ret = 1;
        if (beowulf_->world.HasRoad(pt, dir)) {
            // Check if we exceed the upper traffic limit.
            if (building->GetTraffic().consumed > 0) {
                if ((GetTraffic(pt, dir, 0) + building->GetTraffic().consumed) > UPPER_TRAFFIC_LIMIT)
                    ret += 10; // punish
            }
            if (building->GetTraffic().produced > 0) {
                if ((GetTraffic(pt, dir, 1) + building->GetTraffic().consumed) > UPPER_TRAFFIC_LIMIT)
                    ret += 10; // punish
            }
        } else {
            // New roads cost '5' additional points.
            ret += 5;

            // Building on farmland is also not good.
            MapPoint to = beowulf_->world.GetNeighbour(pt, dir);
            if (nodes_[to].isFarmLand)
                ret += 10;
        }
        return ret;
    });

    if (!ret)
        return false;

    MapPoint cur = start;
    MapPoint subpathStart;
    std::vector<Direction> subpath;
    for (Direction dir : route) {
        if (!beowulf_->world.HasRoad(cur, dir)) {
            if (subpath.empty())
                subpathStart = cur;
            subpath.push_back(dir);
        } else {
            if (!subpath.empty()) {
                beowulf_->world.ConstructRoad(subpathStart, subpath);
                if (buildLocations)
                    buildLocations->Update(subpathStart, subpath.size() + 2);
                subpath.clear();
            }
        }
        cur = nodes_.GetNeighbour(cur, dir);
    }

    if (!subpath.empty()) {
        beowulf_->world.ConstructRoad(subpathStart, subpath);
        if (buildLocations)
            buildLocations->Update(subpathStart, subpath.size() + 2);
    }

    connected.insert(building);
    return true;
}

bool RoadManager::IsConnected(const MapPoint& src, const MapPoint& dst) const
{
    return FindPath(src, beowulf_->world, nullptr,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        if (pt == src)
            return false;
        return beowulf_->world.HasRoad(pt, dir);
    },
    // End
    [&](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == dst;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return beowulf_->world.CalcDistance(pt, dst);
    },
    // Cost
    [&](const MapPoint&, Direction)
    {
        return 1;
    });
}

void RoadManager::OnBuildingNote(const BuildingNote& note)
{
    if (!enabled_)
        return;

    switch (note.type) {
    case BuildingNote::SetBuildingSiteFailed:
    case BuildingNote::Destroyed:
    {
        Building* bld = nullptr;
        RTTR_Assert(!beowulf_->world.GetBuilding(note.pos));

        /*
         * The exact building is not known.
         * But there is a way we can find out which one it was.
         *
         * We check all users on roads attached to the flag position.
         * The one that occurs only once is the correct building.
         * We can even validate that by the building type provided in 'note'.
         */
        MapPoint flag = nodes_.GetNeighbour(note.pos, Direction::SOUTHEAST);
        std::map<const Building*, unsigned> users_counts;
        for (Direction dir : Direction()) {
            for (const Building* user : GetUsers(flag, dir)) {
                if (users_counts.find(user) == users_counts.end())
                    users_counts[user] = 1;
                else
                    users_counts[user]++;
            }
        }

        for (const auto& it : users_counts) {
            if (it.second == 1) {
                RTTR_Assert(bld == nullptr);
                bld = const_cast<Building*>(it.first);
            }
        }

        if (bld)
            UnsetUsage(bld);
    } break;
    case BuildingNote::Captured:
    {
        if (note.player != beowulf_->GetPlayerId())
            break;
        Building* bld = beowulf_->world.GetBuilding(note.pos);
        if (!bld)
            break;
        if (beowulf_->world.IsPointConnected(bld->GetFlag()))
            break;
        Connect(bld);
    } break;
    default:
        break;
    }
}

void RoadManager::OnRoadNote(const RoadNote& note)
{
    if (!enabled_)
        return;

    switch (note.type)
    {
    case RoadNote::Destroyed:
    {
        // Find all buildings that used this road and try to create new connections.
        std::set<const Building*> buildings;
        MapPoint cur = note.pos;
        for (Direction dir : note.route) {
            for (const Building* bld : GetUsers(cur, dir))
                buildings.insert(bld);
            cur = nodes_.GetNeighbour(cur, dir);
        }
        for (const Building* bld : buildings) {
            connected.erase(bld);
            UnsetUsage(bld);
            if (!Connect(bld)) {
                // destroy the construction site
                if (bld->GetState() == Building::UnderConstruction)
                    beowulf_->world.Deconstruct(beowulf_->world.GetBuilding(bld->GetPt()));
            }
        }
    } break;

    case RoadNote::ConstructionFailed:
    {
        // If this road tried to connect a construction site:
        Building* bld = beowulf_->world.GetBuilding(beowulf_->world.GetNeighbour(note.pos, Direction::NORTHWEST));
        if (bld) {
            if (bld->GetState() == Building::UnderConstruction)
                beowulf_->world.Deconstruct(bld);
        }
    } break;
    default:
        break;
    }
}

void RoadManager::SetUsage(const Building* building, const std::vector<Direction>& route)
{
    MapPoint cur = building->GetFlag();
    for (Direction dir : route) {
        SetUsage(building, cur, dir);
        cur = nodes_.GetNeighbour(cur, dir);
    }
}

void RoadManager::SetUsage(const Building* building, const MapPoint& pt, Direction dir)
{
    Direction oppositeDir = OppositeDirection(dir);
    const Building::TrafficExpected& traffic = building->GetTraffic();
    if (dir.toUInt() >= 3) {
        Node& node = nodes_[pt];
        node.users[oppositeDir.toUInt()].push_back(building);
        node.usage[oppositeDir.toUInt()][0] += traffic.produced;
        node.usage[oppositeDir.toUInt()][1] += traffic.consumed;
    } else {
        Node& node = nodes_[nodes_.GetNeighbour(pt, dir)];
        node.users[dir.toUInt()].push_back(building);
        node.usage[dir.toUInt()][1] += traffic.produced;
        node.usage[dir.toUInt()][0] += traffic.consumed;
    }
}

void RoadManager::UnsetUsage(const Building* building)
{
    MapPoint cur = building->GetFlag();
    bool found = true;
    while (found) {
        found = false;
        for (Direction dir : Direction()) {
            std::vector<const Building*>& users = GetUsers(cur, dir);
            if (helpers::contains(users, building)) {
                users.erase(std::remove(users.begin(), users.end(), building), users.end());
                cur = nodes_.GetNeighbour(cur, dir);
                found = true;
                break;
            }
        }
    }
}

std::vector<const Building*>& RoadManager::GetUsers(const MapPoint& pt, Direction dir)
{
    if (dir.toUInt() >= 3)
        return nodes_[pt].users[OppositeDirection(dir).toUInt()];
    else
        return nodes_[nodes_.GetNeighbour(pt, dir)].users[dir.toUInt()];
}

unsigned RoadManager::GetTraffic(const MapPoint& pt, Direction dir, unsigned char d) const
{
    if (dir.toUInt() >= 3)
        return nodes_[pt].usage[OppositeDirection(dir).toUInt()][d];
    else
        return nodes_[nodes_.GetNeighbour(pt, dir)].usage[dir.toUInt()][d];
}

} // namespace beowulf
