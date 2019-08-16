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

#include "ai/beowulf/RoadNetworks.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/Helper.h"

namespace beowulf {

RoadNetworks::RoadNetworks(const World& world)
    : world_(world)
{
}

void RoadNetworks::Resize(const MapExtent& size)
{
    nodes_.Resize(size);
    RTTR_FOREACH_PT(MapPoint, world_.GetSize())
        nodes_[pt] = InvalidRoadNetwork;
}

void RoadNetworks::OnFlagStateChanged(
        const MapPoint& pt,
        FlagState state)
{
    switch (state) {
    case FlagRequested:
    {
        /*
         * When a new flag is requested in can either be on an existing
         * road and thus already have a road network id or it can form a completely
         * new road network because it is not yet attached.
         */
        if (nodes_[pt] == InvalidRoadNetwork)
            nodes_[pt] = next_++;
    } break;

    case FlagDestructionRequested:
    case FlagDoesNotExist:
    {
        if (!world_.IsPointConnected(pt))
            nodes_[pt] = InvalidRoadNetwork;
    } break;

    case FlagFinished:
    default:
        break;
    }
}

rnet_id_t RoadNetworks::Get(const MapPoint& pt) const
{
    if (pt.isValid())
        return nodes_[pt];
    return InvalidRoadNetwork;
}

void RoadNetworks::Detect()
{
    next_ = 0;

    RTTR_FOREACH_PT(MapPoint, nodes_.GetSize())
        nodes_[pt] = InvalidRoadNetwork;

    for (const MapPoint& flag : world_.GetFlags()) {
        if (Get(flag) == InvalidRoadNetwork) {
            FloodFill(world_, flag,
            // condition
            [this](const MapPoint& pt, Direction dir) { return world_.HasRoad(pt, dir); },
            // action
            [this](const MapPoint& pt) { nodes_[pt] = next_; });
            ++next_;
        }
    }
}

} // namespace beowulf
