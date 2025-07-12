// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "world/MapBase.h"
#include "gameTypes/BuildingQuality.h"
#include "nodeObjs/noBase.h"
#include "gameTypes/Direction.h"

#include <vector>

class AIInterface;

namespace beowulf {

// @todo: Rename to RoadBuilder
class RoadBuilder
{
public:
    RoadBuilder(AIInterface& aii,
        const MapPoint& anticipatedPt = MapPoint(),
        BuildingQuality anticipatedBq = BuildingQuality::Nothing);
    ~RoadBuilder();

    // Check if two points could be connected with roads when 'anticipatedPt' would have a building of size 'anticipatedBq'.
    bool CanConnect(const MapPoint& start, const MapPoint& dest) const;
    bool CanConnectToAFlag(const MapPoint& start) const;

    /// Check if we can build a segment from 'pt' in 'dir'. Assumes that we can start at 'pt'.
    bool IsRoadPossible(const MapPoint& pt, Direction dir) const;

    bool HasFlag(const MapPoint& pt) const;
    bool HasRoad(const MapPoint& pt, Direction dir) const;

    bool IsConnected(const MapPoint& pt, bool buildingFlag = false) const;
    //bool IsConnectedTo(const MapPoint& start, const MapPoint& dest) const;

    bool ConnectToNearestFlag(const MapPoint& flagPos);
    // bool ConnectToNearestClient(const MapPoint& flagPos);
    // bool ConnectToNearestWarehouse(const MapPoint& flagPos);

    bool FindConnectionToNearestFlag(const MapPoint& flag, std::vector<Direction>* route);

    // Remove all roads and flags that are not used anymore,
    // connect buildings that are 
    // void Cleanup();

private:
    AIInterface& aii_;
    const MapPoint& anticipatedPt_;
    BuildingQuality anticipatedBq_;

    BuildingQuality GetBQ(const MapPoint& pt) const;
    BlockingManner GetBM(const MapPoint& pt) const;
};

} // namespace beowulf