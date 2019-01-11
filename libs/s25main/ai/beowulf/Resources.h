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
#include "ai/beowulf/Helper.h"

#include "world/NodeMapBase.h"
#include "gameTypes/Resource.h"
#include "notifications/Subscribtion.h"

#include <map>
#include <bitset>

class AIInterface;
class ResourceNote;
class BuildingNote;

namespace beowulf {

class World;

/**
 * @brief Allow querys for resources available at a given point.
 *
 * @todo: Remember visible geologist information from enemies.
 *
 * The resources found will be buffered and kept for 'maxAge' game frames.
 */
class Resources
{
public:
    /// If fow = true only known resources to a player are shown.
    Resources(AIInterface& aii, World& world, bool fow);

    /**
     * For building location selection:
     *
     * Returns the total number of resources that is reachable from the given point
     * which is not used by another building.
     * All resources that are not visible due to FOW can be guessed.
     */
    unsigned GetReachable(
            const MapPoint& pt,
            BResourceType type,
            bool weigthDistance = true,
            bool ignoreOtherBuildings = false,
            bool guess = true);

    /**
     * For total available resources in a region:
     *
     * Returns the number of resources that are reachable from the given point,
     * ignoring all already visited/reachable points in set.
     */
    typedef std::map<MapPoint, std::bitset<BResourceCount>, MapPointComp> VisitedMap;
    std::array<unsigned, BResourceCount> GetReachable(
            const MapPoint& pt,
            VisitedMap& visited,
            bool ignoreOtherBuildings = true,
            bool guess = true);

    /// Get resources at point.
    /// If guess = true and the resources are not known at that point a qualified guess is returned.
    unsigned Get(const MapPoint& pt, BResourceType type, bool guess);

    std::array<unsigned, BResourceCount> GetReachableInRegion(const MapPoint& regionPt);

    void Added(const MapPoint& pt, BuildingType type);
    void Removed(const MapPoint& pt, BuildingType type);

private:
    /// Update the reachable resources at given point for given type.
    void UpdateReachable(const MapPoint& pt, BResourceType type);

    bool IsReachable(const MapPoint& pt, const MapPoint& from, BResourceType type) const;

private:
    AIInterface& aii_;
    World& world_;
    bool fow_;

    // Struct for map information.
    struct Node
    {
        Node();

        // Whether we know which resources lie underneath the earth at this point.
        bool underground_known = false;

        // The resources we know of at this point underground.
        // This is not neccessary if we
        std::array<unsigned, BResourceCount> underground;

        // Whether the given resource is already beeing harvested by a building.
        std::array<unsigned, BResourceCount> harvested;
    };
    NodeMapBase<Node> nodes_;

    void AddResource(Node& node, const MapPoint& dst, const MapPoint& pt, unsigned radius, BResourceType type, unsigned amount);
    unsigned GuessOre(const MapPoint& pt, Resource::Type type) const;

    std::vector<Subscribtion> eventSubscriptions_;
    void OnBuildingNote(const BuildingNote& note);
    void OnResourceNote(const ResourceNote& note);
};

} // namespace beowulf

#endif //! BEOWULF_RESOURCES_H_INCLUDED
