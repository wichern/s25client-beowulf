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
#ifndef BEOWULF_RESOURCE_MAP_H_INCLUDED
#define BEOWULF_RESOURCE_MAP_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/NodeMapBase.h"

class AIInterface;

namespace beowulf {

struct ResourceMapNode
{
    ResourceMapNode();

    // Available resources in range.
    // Range is based on the type of resource.
    unsigned resources[BResourceCount];
    unsigned lastUpdate[BResourceCount];
};

// @todo: rename to "Resources" and make nodes private.
class ResourceMap : public NodeMapBase<ResourceMapNode>
{
public:
    ResourceMap(AIInterface& aii, unsigned maxAge = 100);

    /// Get resources available in range of point pt.
    /// The value will be updated once it is too many gf's old.
    unsigned Get(const MapPoint& pt, BResourceType type);

    /// Get total amount of resource type in player territory.
    unsigned Get(BResourceType type) const;

    /// Update the territorial resource statistics.
    void Refresh();

private:
    void Update(const MapPoint& pt, BResourceType type);

    AIInterface& aii_;
    unsigned maxAge_;

    // Resources in territory
    unsigned resources_[BResourceCount];
};

} // namespace beowulf

#endif //! BEOWULF_RESOURCE_MAP_H_INCLUDED
