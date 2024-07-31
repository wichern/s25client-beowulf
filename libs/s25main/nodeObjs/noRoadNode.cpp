// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "noRoadNode.h"

#include "GamePlayer.h"
#include "RoadSegment.h"
#include "SerializedGameData.h"
#include "world/GameWorld.h"
#include "s25util/warningSuppression.h"

noRoadNode::noRoadNode(const NodalObjectType nop, const MapPoint pos, const unsigned char player)
    : noCoordBase(nop, pos), player(player)
{
    for(const auto dir : helpers::EnumRange<Direction>{})
        routes[dir] = nullptr;
    last_visit = 0;
}

noRoadNode::~noRoadNode() = default;

void noRoadNode::Destroy()
{
    DestroyAllRoads();
    noCoordBase::Destroy();
}

void noRoadNode::Serialize(SerializedGameData& sgd) const
{
    noCoordBase::Serialize(sgd);

    sgd.PushUnsignedChar(player);

    // the trick only seems to work for flags
    if(this->GetGOT() == GO_Type::Flag)
    {
        // this is a trick:
        // -> initialize routes for flag with nullptr
        // -> RoadSegment will set these later
        for(const auto i : helpers::EnumRange<Direction>{})
        {
            RTTR_UNUSED(i);
            sgd.PushObject(static_cast<GameObject*>(nullptr), true);
        }
    } else
    {
        for(const auto dir : helpers::EnumRange<Direction>{})
        {
            sgd.PushObject(routes[dir], true);
        }
    }
}

noRoadNode::noRoadNode(SerializedGameData& sgd, const unsigned obj_id)
    : noCoordBase(sgd, obj_id), player(sgd.PopUnsignedChar())
{
    for(const auto dir : helpers::EnumRange<Direction>{})
    {
        routes[dir] = sgd.PopObject<RoadSegment>(GO_Type::Roadsegment);
    }

    last_visit = 0;
}

void noRoadNode::UpgradeRoad(const Direction dir) const
{
    if(GetRoute(dir))
        GetRoute(dir)->UpgradeDonkeyRoad();
}

void noRoadNode::DestroyRoad(const Direction dir)
{
    RoadSegment* route = GetRoute(dir);
    if(!route)
        return;
    MapPoint t = route->GetF1()->GetPos();
    for(unsigned z = 0; z < route->GetLength(); ++z)
    {
        world->SetPointRoad(t, route->GetRoute(z), PointRoad::None);
        world->RecalcBQForRoad(t);
        t = world->GetNeighbour(t, route->GetRoute(z));
    }

    noRoadNode* otherFlag;

    if(route->GetF1() == this)
        otherFlag = route->GetF2();
    else
        otherFlag = route->GetF1();

    for(const auto z : helpers::EnumRange<Direction>{})
    {
        if(otherFlag->routes[z] == route)
        {
            otherFlag->routes[z] = nullptr;
            break;
        }
    }

    SetRoute(dir, nullptr);
    UpdateVirtualRoadSegment(dir);

    route->Destroy();
    delete route;

    // Spieler Bescheid sagen
    world->GetPlayer(player).RoadDestroyed();
}

/// Vernichtet Alle Straße um diesen Knoten
void noRoadNode::DestroyAllRoads()
{
    // Alle Straßen um mich herum zerstören
    for(const auto dir : helpers::EnumRange<Direction>{})
        DestroyRoad(dir);
}

#include "PointOutput.h"
#include "gameTypes/GameTypesOutput.h"
#include <iostream>

void noRoadNode::UpdateVirtualRoadSegment(const Direction dir)
{
    std::cout << "UpdateVirtualRoadSegment(" << GetPos() << ", " << dir << ")" << std::endl;

    unsigned num_routes = 0;
    for(const auto d : helpers::EnumRange<Direction>{})
    {
        if(routes[d])
            num_routes++;
    }

    if(num_routes == 0)
        return;

    if(num_routes == 1)
    {
        // if this would have a virtual segment, it would be set by the case below
        return;
    }

    if(num_routes == 2)
    {
        if(vroutes[dir] == nullptr)
        {
            // create vroute
            std::shared_ptr<VirtualRoadSegment> vroute = std::make_shared<VirtualRoadSegment>();
            vroute->f1 = FollowVRoute(dir, vroute);

            Direction otherDir = dir;
            for(const auto d : helpers::EnumRange<Direction>{})
            {
                if(d != dir)
                {
                    otherDir = d;
                    break;
                }
            }
            RTTR_Assert(otherDir != dir);
            vroute->f2 = FollowVRoute(otherDir, vroute);

            vroutes[dir] = vroute;
            return;
        }

        // Are we at the end of a vroute?
        for(const auto d : helpers::EnumRange<Direction>{})
        {
            std::shared_ptr<VirtualRoadSegment> vroute = vroutes[d];
            if(vroute == nullptr)
                continue;
            if(vroute->f1 == this || vroute->f2 == this)
            {
                // extend vroute into dir
            }
            break; // can only be one
        }
    }

    // num_routes > 2

    const std::shared_ptr<VirtualRoadSegment> vroute = vroutes[dir];
    if(vroute)
    {
        if(vroute->f1 != this && vroute->f2 != this)
        {
            // split this virtual segment
        }
    }
}

noRoadNode* noRoadNode::FollowVRoute(Direction dir, std::shared_ptr<VirtualRoadSegment> vroute)
{
    // We have to go from this node into dir, until we reach a flag that has != 2 routes.
    // All flags on that path are part of the vroute

    RoadSegment* segment = routes[dir];
    noRoadNode* node = segment->GetF1() == this ? segment->GetF2() : segment->GetF1();
    while(true)
    {
        RTTR_Assert(node);

        unsigned num_routes = 0;
        for(const auto d : helpers::EnumRange<Direction>{})
        {
            if(routes[d])
                num_routes++;
        }

        // @todo: Where do we have to place the roads?

        if(num_routes != 2)
        {
            break;
        }
    }

    return node;
}
