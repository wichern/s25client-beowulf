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
#ifndef BEOWULF_ROADNETWORKS_H_INCLUDED
#define BEOWULF_ROADNETWORKS_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/NodeMapBase.h"

namespace beowulf {

class World;

/**
 * @brief Detect connected flags.
 */
class RoadNetworks
{
public:
    RoadNetworks(const World& world);
    void Resize(const MapExtent& size);

    rnet_id_t Get(const MapPoint& pt) const;
    void OnFlagStateChanged(const MapPoint& pt, FlagState state);
    void Detect();

private:
    NodeMapBase<rnet_id_t> nodes_;
    const World& world_;
    rnet_id_t next_ = 0;
};

} // namespace beowulf

#endif //! BEOWULF_ROADNETWORKS_H_INCLUDED
