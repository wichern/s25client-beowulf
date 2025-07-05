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
class RoadManager
{
public:
    RoadManager(AIInterface& aii,
        const MapPoint& anticipatedPt = MapPoint(),
        BuildingQuality anticipatedBq = BuildingQuality::Nothing);
    ~RoadManager();

    // Check if two points could be connected with roads when 'anticipatedPt' would have a building of size 'anticipatedBq'.
    bool CanConnect(const MapPoint& start, const MapPoint& dest) const;

    /// Check if we can build a segment from 'pt' in 'dir'. Assumes that we can start at 'pt'.
    bool IsRoadPossible(const MapPoint& pt, Direction dir) const;

    bool HasFlag(const MapPoint& pt) const;
    bool HasRoad(const MapPoint& pt, Direction dir) const;

    // @todo: Connection strategies...
    //
    // Scenario #1: We place a building site
    //  Option #1: Connect it to the next closest flag
    //  Option #2: Connect it to the closest building that needs the resource from this building
    //  Option #3: Connect to next warehouse
    //
    // Scenario #2: A road was destroyed and buildings are not connected anymore
    //  Remove all roads that are not used anymore
    //  Option #1: React on all events where changes can happen
    //  Option #2: Do regular clean-up intervals
    //  
    // void Connect(const MapPoint& src, const MapPoint& dest);
    // void Cleanup();

private:
    AIInterface& aii_;
    const MapPoint& anticipatedPt_;
    BuildingQuality anticipatedBq_;

    BuildingQuality GetBQ(const MapPoint& pt) const;
    BlockingManner GetBM(const MapPoint& pt) const;
};

} // namespace beowulf