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

#include "ai/beowulf/BuildingPlannerSimple.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/ResourceMap.h"

#include "ai/AIInterface.h"

#include <limits>
#include <cmath> /* exp() */
#include <algorithm> /* std::min */

namespace beowulf {

BuildingPlannerSimple::BuildingPlannerSimple(
        AIInterface& aii,
        Buildings& buildings,
        ResourceMap& resources,
        rnet_id_t rnet)
    : BuildingPlannerBase(aii, buildings, resources, rnet)
{

}

BuildingPlannerSimple::~BuildingPlannerSimple()
{

}

void BuildingPlannerSimple::Init(const std::vector<Building*>& requests)
{
    requests_ = requests;
}

void BuildingPlannerSimple::Search()
{
    /*
     * Sorting order
     *
     *  a) Buildings with fixed position before those where we need to find a position.
     *  b) Buildings with their goods destinations in a production group and already
     *     placed before those where it is not already placed.
     *  c) Buildings with high quality before low quality.
     */
    std::stable_sort(requests_.begin(), requests_.end(),
              [this](const Building* l, const Building* r)
    {
        if (l->GetPt().isValid() && !r->GetPt().isValid())
            return true;

        if (l->IsProduction() && r->IsProduction()) {
            bool ldest = buildings_.HasGoodsDest(l);
            bool rdest = buildings_.HasGoodsDest(r);
            if (ldest && !rdest)
                return true;
            if (!ldest && rdest)
                return false;
        }

        return l->GetQuality() > r->GetQuality();
    });

    searches_++;
}

void BuildingPlannerSimple::Execute()
{
    for (Building* building : requests_) {
        MapPoint pt;
        std::vector<Direction> route;
        Place(building, pt, route, true);
    }
}

unsigned BuildingPlannerSimple::GetSearches() const
{
    return searches_;
}

unsigned BuildingPlannerSimple::GetMaxSearches() const
{
    return 1;
}

} // namespace beowulf
