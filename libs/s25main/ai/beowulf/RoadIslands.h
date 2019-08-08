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
#ifndef BEOWULF_ROADISLANDS_H_INCLUDED
#define BEOWULF_ROADISLANDS_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/NodeMapBase.h"

namespace beowulf {

class Buildings;

class RoadNetworks
{
private:
    NodeMapBase<rnet_id_t> islands_;
    const MapBase& world_;
    rnet_id_t next_ = 0;

public:
    RoadNetworks(const MapBase& world);
    rnet_id_t Get(const MapPoint& pos) const;

    void OnFlagStateChanged(const Buildings& buildings, const MapPoint& pos, FlagState state);
    void Detect(const Buildings& buildings);
};

} // namespace beowulf

#endif //! BEOWULF_ROADISLANDS_H_INCLUDED
