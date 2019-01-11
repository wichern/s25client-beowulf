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

#include "ai/beowulf/Resources.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/ProductionConsts.h"

#include "ai/AIInterface.h"
#include "world/GameWorldBase.h"
#include "nodeObjs/noTree.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noAnimal.h"
#include "gameData/GameConsts.h"
#include "EventManager.h"
#include "notifications/BuildingNote.h"
#include "notifications/ResourceNote.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>

namespace beowulf {

// Map Resource to BResourceType
static const BResourceType c_map[] = {
    BResourceCount, // nothing
    BResourceIron,
    BResourceGold,
    BResourceCoal,
    BResourceGranite,
    BResourceWater,
    BResourceFish
};

static const Resource::Type S_map_resource_types[BResourceCount] {
    Resource::Iron, Resource::Gold, Resource::Coal, Resource::Granite, Resource::Water,
    Resource::Nothing, Resource::Nothing, Resource::Fish, Resource::Nothing, Resource::Nothing,
    Resource::Nothing
};

static const unsigned S_resource_radius[BResourceCount] = {
    MINER_RADIUS, MINER_RADIUS, MINER_RADIUS, MINER_RADIUS, 1, FARMER_RADIUS, 6,
    FISHER_RADIUS, 20, WOODCUTTER_RADIUS, STONEMASON_RADIUS
};

Resources::Node::Node()
{
    underground.fill(0);
    harvested.fill(0);
}

Resources::Resources(AIInterface& aii, World& world, bool fow)
    : aii_(aii),
      world_(world),
      fow_(fow)
{
    nodes_.Resize(aii.gwb.GetSize());

    NotificationManager& notifications = aii.gwb.GetNotifications();
    eventSubscriptions_.push_back(notifications.subscribe<BuildingNote>(
        boost::lambda::if_(boost::lambda::bind(&BuildingNote::player, boost::lambda::_1) == aii_.GetPlayerId())
                                      [boost::lambda::bind(&Resources::OnBuildingNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<ResourceNote>(
        boost::lambda::if_(boost::lambda::bind(&ResourceNote::player, boost::lambda::_1) == aii_.GetPlayerId())
                                      [boost::lambda::bind(&Resources::OnResourceNote, this, boost::lambda::_1)]));
}

unsigned Resources::GetReachable(
        const MapPoint& pt,
        BResourceType type,
        bool weigthDistance,
        bool ignoreOtherBuildings,
        bool guess)
{
    unsigned ret = 0;
    const Node& node = nodes_[pt];

    unsigned radius = S_resource_radius[type];
    nodes_.VisitPointsInRadius(pt, radius, [&](const MapPoint& p)
    {
        if (!ignoreOtherBuildings && node.harvested[type] > 0)
            return;

        unsigned count = Get(p, type, guess);
        if (count > 0) {
            if (IsReachable(p, pt, type)) {
                if (weigthDistance) {
                    ret += ((radius + 1) - nodes_.CalcDistance(pt, p)) * count;
                } else {
                    ret += count;
                }
            }
        }
    }, true);

    return ret;
}

std::array<unsigned, BResourceCount> Resources::GetReachable(
        const MapPoint& pt,
        VisitedMap& visited,
        bool ignoreOtherBuildings,
        bool guess)
{
    std::array<unsigned, BResourceCount> ret;
    ret.fill(0);

    const Node& node = nodes_[pt];

    for (unsigned t = 0; t < BResourceCount; ++t) {
        BResourceType type = static_cast<BResourceType>(t);

        unsigned radius = S_resource_radius[type];
        nodes_.VisitPointsInRadius(pt, radius, [&](const MapPoint& p)
        {
            std::bitset<BResourceCount>& visitedResource = visited[p];
            if (visitedResource[type])
                return;
            visitedResource[type] = true;

            if (!ignoreOtherBuildings && node.harvested[type] > 0)
                return;

            unsigned count = Get(p, type, guess);
            if (count > 0) {
                if (IsReachable(p, pt, type)) {
                    ret[type] += count;
                }
            }
        }, true);
    }

    return ret;
}

unsigned Resources::Get(const MapPoint& pt, BResourceType type, bool guess)
{
    Node& node = nodes_[pt];

    const Resource res = aii_.gwb.GetNode(pt).resources;

    switch (type) {
    case BResourceIron:
    case BResourceGold:
    case BResourceCoal:
    case BResourceGranite:
    {
        if (!fow_ || node.underground_known)
            return res.getType() == S_map_resource_types[type] ? res.getAmount() : 0;
        if (guess && aii_.gwb.IsMineable(pt))
            return GuessOre(pt, S_map_resource_types[type]);
        return 0;
    }
    case BResourceWater:
    {
        // We know where water is.
        return res.getType() == Resource::Water ? res.getAmount() : 0;
    }
    case BResourcePlantSpace_2:
    case BResourcePlantSpace_6:
    {
        if (!fow_ || aii_.IsVisible(pt))
            return aii_.gwb.IsPlantSpace(pt) ? 1 : 0;
        if (guess)
            return 0;
        return 0;
    }
    case BResourceFish:
    {
        if (!fow_ || node.underground_known)
            return res.getType() == S_map_resource_types[type] ? res.getAmount() : 0;
        if (guess)
            return aii_.gwb.IsWaterPoint(pt) ? 1 : 0;
        return 0;
    }
    case BResourceHuntableAnimals:
    {
        unsigned ret = 0;
        for (const noBase* fig : aii_.gwb.GetFigures(pt)) {
            if (fig->GetType() == NOP_ANIMAL && static_cast<const noAnimal*>(fig)->CanHunted())
                ret++;
        }
        return ret;
    }
    case BResourceWood:
    {
        if (!fow_ || aii_.IsVisible(pt))
            return aii_.gwb.IsWalkable(pt)
                    && aii_.gwb.GetNO(pt)->GetType() == NOP_TREE
                    && aii_.gwb.GetSpecObj<noTree>(pt)->ProducesWood() ? 1 : 0;
        if (guess)
            return 0;
        return 0;
    }
    case BResourceStone:
    {
        if (!fow_ || aii_.IsVisible(pt)) {
            if (aii_.gwb.GetNO(pt)->GetType() != NOP_GRANITE)
                return 0;
            if (!aii_.gwb.IsWalkable(pt))
                return 0;
            return static_cast<unsigned>(aii_.gwb.GetSpecObj<noGranite>(pt)->GetAmount());
        }
        if (guess)
            return 0;
        return 0;
    }
    default: RTTR_Assert(false);
    }

    return 0;
}

void Resources::Added(const MapPoint& pt, BuildingType type)
{
    BResourceType resourceType = REQUIRED_RESOURCES[type];
    if (resourceType == BResourceCount)
        return;

    unsigned radius = S_resource_radius[resourceType];
    nodes_.VisitPointsInRadius(pt, radius, [&](const MapPoint& p)
    {
        nodes_[p].harvested[resourceType]++;
    }, false);
}

void Resources::Removed(const MapPoint& pt, BuildingType type)
{
    BResourceType resourceType = REQUIRED_RESOURCES[type];
    if (resourceType == BResourceCount)
        return;

    unsigned radius = S_resource_radius[resourceType];
    nodes_.VisitPointsInRadius(pt, radius, [&](const MapPoint& p)
    {
        RTTR_Assert(nodes_[p].harvested[resourceType] > 0);
        nodes_[p].harvested[resourceType]--;
    }, false);
}

bool Resources::IsReachable(
        const MapPoint& pt,
        const MapPoint& from,
        BResourceType type) const
{
    if (type == BResourceFish) {
        // Try to find a path to one spot next to the fish.
        for (Direction dir : Direction())  {
            MapPoint fishNeighbour = nodes_.GetNeighbour(pt, dir);
            if (!aii_.gwb.IsWalkable(fishNeighbour))
                continue;
            if (aii_.gwb.FindHumanPath(from, fishNeighbour, 10) != INVALID_DIR) {
                return true;
            }
        }

        return false;
    }

    if (type == BResourceHuntableAnimals || type == BResourceWood || type == BResourceStone) {
        unsigned max = type == BResourceHuntableAnimals ? 50 : 20;
        return from == pt || aii_.gwb.FindHumanPath(from, pt, max) != INVALID_DIR;
    }

    return true;
}

unsigned Resources::GuessOre(const MapPoint& pt, Resource::Type type) const
{
    unsigned total = 0;
    unsigned neighbours = 0;
    nodes_.VisitPointsInRadius(pt, 1, [&](const MapPoint& p)
    {
        if (nodes_[p].underground_known && aii_.gwb.IsMineable(pt)) {
            neighbours++;
            const Resource& res = aii_.gwb.GetNode(p).resources;
            if (res.getType() == type)
                total += res.getAmount();
        }
    }, false);

    // If we know nothing about neighbouring ores we just assume that there is one resource of this type.
    if (0 == neighbours)
        return 1;

    return std::max(1u, total / neighbours); // At least expect one mineral.
}

void Resources::OnBuildingNote(const BuildingNote& note)
{
    if (note.type == BuildingNote::NoRessources) {
        switch (note.bld) {
        case BLD_FISHERY:
            // We can say for sure that there is no more fish around here.
            for (const MapPoint& p : nodes_.GetPointsInRadius(note.pos, FISHER_RADIUS)) {
                if (aii_.gwb.IsWaterPoint(p))
                    nodes_[p].underground_known = true;
            }
            break;
        case BLD_COALMINE:
        case BLD_GOLDMINE:
        case BLD_GRANITEMINE:
        case BLD_IRONMINE:
            /*
             * We cannot do anything here, because we can only safely say that there is no
             * more of the corresponding ore here but not if there are other ores on
             * points we did not discover earlier.
             */
            break;
        default: break;
        }
    }
}

void Resources::OnResourceNote(const ResourceNote& note)
{
    // Geologist found resource (can be zero)
    Node& node = nodes_[note.pos];
    node.underground_known = true;
}

} // namespace beowulf
