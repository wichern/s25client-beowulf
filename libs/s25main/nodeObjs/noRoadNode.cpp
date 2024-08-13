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

    auto* neighbour = GetNeighbour(dir);
    SetRoute(dir, nullptr);

    route->Destroy();
    delete route;

    if (neighbour)
        neighbour->UpdateVirtualRoadSegment();

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

void noRoadNode::UpdateVirtualRoadSegment()
{
    // std::cout << "UpdateVirtualRoadSegment(" << GetPos() << ", " << dir << ")" << std::endl;

    unsigned num_routes = 0;
    for(const auto d : helpers::EnumRange<Direction>{})
    {
        if(routes[d])
            num_routes++;
        else
            vroutes[d] = nullptr;
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
        std::shared_ptr<VirtualRoadSegment> vroute; // = 

        for(const Direction d : helpers::EnumRange<Direction>{}) {
            if (routes[d] && vroutes[d] == nullptr) {
                if (vroute == nullptr) {
                    vroute = std::make_shared<VirtualRoadSegment>();
                    vroute->f1 = FollowVRoute(d, vroute);
                } else {
                    vroute->f2 = FollowVRoute(d, vroute);
                }
                vroutes[d] = vroute;
            }
        }

        return;
    }

    // num_routes > 2
    for(const Direction d : helpers::EnumRange<Direction>{})
    {
        const std::shared_ptr<VirtualRoadSegment> vroute = vroutes[d];
        if(vroute)
        {
            // are we in the middle of a virtual route?
            if(vroute->f1 != this && vroute->f2 != this)
            {
                // Split Route
                RemoveVRoute(d, vroute);
            }
            GetNeighbour(d)->UpdateVirtualRoadSegment();
        }
    }
}

noRoadNode* noRoadNode::FollowVRoute(Direction dir, std::shared_ptr<VirtualRoadSegment> vroute)
{
    // We have to go from this node into dir, until we reach a flag that has != 2 routes.
    // All flags on that path are part of the vroute

    noRoadNode* prevNode = this;
    noRoadNode* node = GetNeighbour(dir);
    while(true)
    {
        RTTR_Assert(node);

        unsigned num_routes = 0;
        Direction dir2prev;
        Direction dir2next;

        for(const auto d : helpers::EnumRange<Direction>{})
        {
            RoadSegment* route = node->routes[d];
            if(route)
            {
                num_routes++;
                if(route->GetF1() == prevNode || route->GetF2() == prevNode)
                {
                    dir2prev = d;
                } else
                {
                    dir2next = d;
                }
            }
        }

        std::cout << "(" << node->GetPos() << ", " << dir2prev << ") (next: " << dir2next << ")" << std::endl;
        node->vroutes[dir2prev] = vroute;

        if(num_routes != 2)
        {
            break;
        }

        node->vroutes[dir2next] = vroute;
        prevNode = node;
        node = node->GetNeighbour(dir2next);
    }

    return node;
}

void noRoadNode::RemoveVRoute(Direction dir, std::shared_ptr<VirtualRoadSegment> vroute)
{
    // In order to remove the VRoute, we walk till the end
    noRoadNode* node = GetNeighbour(dir);
    vroutes[dir] = nullptr;
    noRoadNode* lastNode = this;
    bool hasMore = true;
    while(hasMore)
    {
        hasMore = false;
        for(const Direction d : helpers::EnumRange<Direction>{})
        {
            if(node->vroutes[d] == vroute)
            {
                node->vroutes[d] = nullptr;
                noRoadNode* next = node->GetNeighbour(d);
                if(next == lastNode)
                    continue;
                node = next;
                hasMore = true;
            }
        }
    }
}
