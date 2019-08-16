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
#ifndef BEOWULF_RESOURCES_H_INCLUDED
#define BEOWULF_RESOURCES_H_INCLUDED

#include "ai/beowulf/Types.h"

#include "world/NodeMapBase.h"

class AIInterface;

namespace beowulf {

/**
 * @brief Allow querys for resources available at a given point.
 *
 * The resources found will be buffered and kept for 'maxAge' game frames.
 */
class Resources
{
public:
    Resources(AIInterface& aii, unsigned maxAge = 100);

    /// Get resources available in range of point pt.
    /// The value will be updated once it is too many gf's old.
    unsigned Get(const MapPoint& pt, BResourceType type);

    /// Get total amount of resource type in player territory.
    unsigned Get(BResourceType type) const;

    /// Update the territorial resource statistics.
    void Refresh();

private:
    void Update(const MapPoint& pt, BResourceType type);

private:
    AIInterface& aii_;

    struct Node
    {
        Node();

        // Available resources in range.
        // Range is based on the type of resource.
        unsigned resources[BResourceCount];
        unsigned lastUpdate[BResourceCount];
    };
    NodeMapBase<Node> nodes_;

    // Maximum GF before a resource needs recalculation.
    unsigned maxAge_;

    // Resources in territory
    unsigned resources_[BResourceCount];
};

} // namespace beowulf

#endif //! BEOWULF_RESOURCES_H_INCLUDED
