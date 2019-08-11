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

#include "ai/beowulf/RoadIslands.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/Helper.h"

namespace beowulf {

RoadNetworks::RoadNetworks(const MapBase& world)
    : world_(world)
{
    islands_.Resize(world.GetSize());
    RTTR_FOREACH_PT(MapPoint, world.GetSize())
        islands_[pt] = InvalidRoadNetwork;
}

void RoadNetworks::OnFlagStateChanged(
        const Buildings& buildings,
        const MapPoint& pos,
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
        if (islands_[pos] == InvalidRoadNetwork)
            islands_[pos] = next_++;
    } break;

    case FlagFinished:
    {
//        //RTTR_Assert(state == FlagFinished || islands_[pos] == InvalidRoadNetwork);
//        rnet_id_t rnet = islands_[pos];
//        (void)rnet;
//        RTTR_Assert((buildings.IsPointConnected(pos) && islands_[pos] != InvalidRoadNetwork) || islands_[pos] == InvalidRoadNetwork);

//        if (buildings.IsPointConnected(pos)) {
//            // If the flag is already connected to a road we should have
//            // already defined the road network id.
//            // ... nothing to be done.

//            // If the flag is already connected to a road it copies the island of
//            // the flag on the other end of the road.
////            MapPoint cur = pos;
////            unsigned prevDir = Direction::COUNT;
////            for (unsigned limit = 0; limit < 100; ++limit) {
////                for (unsigned dir = 0; dir < Direction::COUNT; ++dir) {
////                    if (dir != prevDir &&
////                            buildings.HasRoad(cur, Direction(dir)) &&
////                            !buildings.HasBuilding(islands_.GetNeighbour(cur, Direction(dir))))
////                    {
////                        cur = world_.GetNeighbour(cur, Direction(dir));
////                        if (buildings.HasFlag(cur)) {
////                            islands_[pos] = Get(cur);
////                            return;
////                        }
////                        prevDir = OppositeDirection(Direction(dir)).toUInt();
////                        break;
////                    }
////                }
////            }
//        } else {
//            // Not connected to a road means that we need to create a new island.
//        }
    } break;

    case FlagDestructionRequested:
    case FlagDoesNotExist:
    {
        if (!buildings.IsPointConnected(pos))
            islands_[pos] = InvalidRoadNetwork;
    } break;
    }
}

rnet_id_t RoadNetworks::Get(const MapPoint& pos) const
{
    if (pos.isValid())
        return islands_[pos];
    return InvalidRoadNetwork;
}

void RoadNetworks::Detect(const Buildings& buildings)
{
    next_ = 0;

    RTTR_FOREACH_PT(MapPoint, islands_.GetSize())
        islands_[pt] = InvalidRoadNetwork;
//    for (const MapPoint& flag : buildings.GetFlags())
//        islands_[flag] = InvalidRoadNetwork;

    for (const MapPoint& flag : buildings.GetFlags()) {
        if (Get(flag) == InvalidRoadNetwork) {
            FloodFill(world_, flag,
            // condition
            [&](const MapPoint& pos, Direction dir)
            { return buildings.HasRoad(pos, dir); },
            // action
            [&](const MapPoint& pos)
            {
                //if (buildings.HasFlag(pos) || buildings.GetBQC().GetBQ(pos) >= BQ_FLAG)
                    islands_[pos] = next_;
            });
            ++next_;
        }
    }
}

} // namespace beowulf
