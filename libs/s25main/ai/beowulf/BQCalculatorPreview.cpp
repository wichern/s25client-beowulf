// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ai/beowulf/BQCalculatorPreview.h"
#include "world/World.h"

namespace beowulf {

BQCalculatorPreview::BQCalculatorPreview(const World& gw, const MapPoint& previewPt, BuildingQuality previewBq)
: BQCalculator(gw)
, previewPt_(previewPt)
, previewBq_(previewBq)
{

}

BlockingManner BQCalculatorPreview::GetBM(const MapPoint& pt) const
{
    if (pt == previewPt_)
        return BlockingManner::Building;

    if (pt == world.GetNeighbour(previewPt_, Direction::SouthEast))
        return BlockingManner::Flag;
    
    // Check for castle extensions
    if (previewBq_ == BuildingQuality::Castle)
        for (auto dir = Direction::East; dir > Direction::NorthEast; dir = dir + 1)
            if (world.GetNeighbour(pt, dir) == previewPt_)
                return BlockingManner::Single;

    return world.GetNO(pt)->GetBM();
}

} // namespace beowulf