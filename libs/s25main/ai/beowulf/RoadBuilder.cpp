// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/RoadBuilder.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/BQCalculatorPreview.h"
#include "ai/beowulf/RoadBuilder.h"
#include "ai/AIInterface.h"
#include "nodeObjs/noFlag.h"
#include "pathfinding/PathConditionRoad.h"

namespace beowulf {

RoadBuilder::RoadBuilder(
    AIInterface& aii,
    const MapPoint& anticipatedPt,
    BuildingQuality anticipatedBq)
: aii_(aii)
, anticipatedPt_(anticipatedPt)
, anticipatedBq_(anticipatedBq)
{
}

RoadBuilder::~RoadBuilder()
{

}

bool RoadBuilder::CanConnect(
    const MapPoint& start,
    const MapPoint& dest) const
{
    if (start == dest)
        return true;

    return beowulf::FindPath(start, aii_.gwb, nullptr,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        return HasRoad(pt, dir) || IsRoadPossible(pt, dir);
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
        return aii_.gwb.CalcDistance(pt, dest);
    },
    // Cost
    [&](const MapPoint& /*pt*/, Direction /*dir*/)
    {
        return 1;
    });
}

bool RoadBuilder::CanConnectToAFlag(const MapPoint& start) const
{
    return beowulf::FindPath(start, aii_.gwb, nullptr,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        return IsRoadPossible(pt, dir);
    },
    // End
    [&](const MapPoint& pt)
    {
        auto* no = aii_.gwb.GetNO(pt);
        return no && no->GetType() == NodalObjectType::Flag;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return aii_.gwb.CalcDistance(pt, start);
    },
    // Cost
    [&](const MapPoint& /*pt*/, Direction /*dir*/)
    {
        return 1;
    });
}

bool RoadBuilder::IsRoadPossible(
    const MapPoint& pt,
    Direction dir) const
{
    MapPoint dest = aii_.gwb.GetNeighbour(pt, dir);

    // If 'pt' already has more than one road and we can't place a flag then a road is impossible.
    if (!HasFlag(pt)) {
        for (const auto d : helpers::EnumRange<Direction>{}) {
            if (HasRoad(pt, d)) {
                if (GetBQ(pt) < BuildingQuality::Flag)
                    return false;
                break;
            }
        }
    }

    if (HasFlag(dest)) {
        // Do we need to place a flag at pt?
        unsigned roads = 0;
        for (const auto d : helpers::EnumRange<Direction>{}) {
            if (HasRoad(pt, d))
                roads++;
        }
        if (roads >= 2)
            return false;

        return true;
    }

    // If dest already has roads, we need to check whether we can place a flag
    // and not need to place a flag from where we come.
    for (const auto d : helpers::EnumRange<Direction>{}) {
        if (HasRoad(dest, d)) {
            // Can we place a flag at dest?
            if (GetBQ(dest) < BuildingQuality::Flag)
                return false;

            // Do we need to place a flag at pt?
            unsigned roads = 0;
            for (const auto d2 : helpers::EnumRange<Direction>{}) {
                if (HasRoad(pt, d2))
                    roads++;
            }
            if (roads >= 2)
                return false;

            return true;
        }
    }

    if (GetBM(dest) != BlockingManner::None)
        return false;

    if (!aii_.gwb.IsPlayerTerritory(dest))
        return false;

    return aii_.gwb.IsRoadAvailable(false, dest);
}


bool RoadBuilder::HasFlag(const MapPoint& pt) const
{
    return aii_.gwb.GetSpecObj<noFlag>(pt) != nullptr;
}

bool RoadBuilder::HasRoad(const MapPoint& pt, Direction dir) const
{
    PointRoad road = aii_.gwb.GetPointRoad(pt, dir);
    return road == PointRoad::Normal || road == PointRoad::Donkey;
}

BuildingQuality RoadBuilder::GetBQ(const MapPoint& pt) const
{
    BQCalculatorPreview bqc(aii_.gwb, anticipatedPt_, anticipatedBq_);
    return bqc(pt, [&](const MapPoint& pos) { return aii_.gwb.IsOnRoad(pos); });
}

BlockingManner RoadBuilder::GetBM(const MapPoint& pt) const
{
    if (pt == anticipatedPt_)
        return BlockingManner::Building;
    
    // Check for castle extensions
    if (anticipatedBq_ == BuildingQuality::Castle)
        for (auto dir = Direction::East; dir > Direction::NorthEast; dir = dir + 1)
            if (aii_.gwb.GetNeighbour(pt, dir) == anticipatedPt_)
                return BlockingManner::Single;

    return aii_.gwb.GetNO(pt)->GetBM();
}

bool RoadBuilder::ConnectToNearestFlag(const MapPoint& flagPos)
{
    RTTR_Assert(flagPos.isValid());

    std::vector<Direction> route;
    bool ret = beowulf::FindPath(flagPos, aii_.gwb, &route,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        return IsRoadPossible(pt, dir);
    },
    // End
    [&](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        auto* no = aii_.gwb.GetNO(pt);
        return no && no->GetType() == NodalObjectType::Flag;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return aii_.gwb.CalcDistance(pt, flagPos);
    },
    // Cost
    [&](const MapPoint& /*pt*/, Direction /*dir*/)
    {
        return 1;
    });

    if (ret)
        return aii_.BuildRoad(flagPos, false, route);

    return ret;
}

bool RoadBuilder::IsConnected(const MapPoint& pt, bool buildingFlag) const
{
    for (const auto dir : helpers::enumRange<Direction>())
        if ((!buildingFlag || dir != Direction::NorthWest) && HasRoad(pt, dir))
            return true;
    return false;
}

} // namespace beowulf