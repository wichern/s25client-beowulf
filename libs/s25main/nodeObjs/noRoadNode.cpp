// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "noRoadNode.h"

#include "GamePlayer.h"
#include "RoadSegment.h"
#include "SerializedGameData.h"
#include "pathfinding/RoadPathFinder.h"
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
    world->GetRoadPathFinder().OnNodeDestroyed(this, world);
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

void noRoadNode::DestroyRoad(const Direction dir, bool updateDstar)
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

    // @todo: can use SetRoute(route->GetF1(), route->GetF2(), *route, nullptr) here?
    noRoadNode* otherFlag;
    if(route->GetF1() == this)
        otherFlag = route->GetF2();
    else
        otherFlag = route->GetF1();

    for(const auto z : helpers::EnumRange<Direction>{})
    {
        if(otherFlag->routes[z] == route)
        {
            otherFlag->SetRoute(z, nullptr);
            break;
        }
    }

    SetRoute(dir, nullptr);

    if (updateDstar) {
        route->GetF1()->OnWaresCostChanged();
        route->GetF2()->OnWaresCostChanged();
    }

    route->Destroy();
    delete route;

    // Spieler Bescheid sagen
    world->GetPlayer(player).RoadDestroyed();
}

/// Vernichtet Alle Straße um diesen Knoten
void noRoadNode::DestroyAllRoads()
{
    // Alle Straßen um mich herum zerstören
    for(const auto z : helpers::EnumRange<Direction>{})
    {
        if(routes[z])
        {
            if (routes[z]->GetF1() == this)
                routes[z]->GetF2()->OnWaresCostChanged();
            if (routes[z]->GetF2() == this)
                routes[z]->GetF1()->OnWaresCostChanged();
        }
    }
    OnWaresCostChanged();

    for(const auto dir : helpers::EnumRange<Direction>{})
        DestroyRoad(dir, false);
}

void noRoadNode::OnWaresCostChanged()
{
    for (auto& [goal_pos, _] : dstar.map)
        world->GetRoadPathFinder().MarkNodeDirty(goal_pos, this);
}

void noRoadNode::SetRoute(noRoadNode* n1, noRoadNode* n2, const std::vector<Direction>& route, RoadSegment* segment)
{
    n1->SetRoute(route.front(), segment);
    n2->SetRoute(route.back() + 3u, segment);

    // For all of one of the nodes goals, add both nodes to the dirty list.
    for (auto& [goal_pos, _] : n1->dstar.map) {
        world->GetRoadPathFinder().MarkNodeDirty(goal_pos, n1);
        world->GetRoadPathFinder().MarkNodeDirty(goal_pos, n2);
    }
    if (n1 != n2) {
        for (auto& [goal_pos, _] : n2->dstar.map) {
            world->GetRoadPathFinder().MarkNodeDirty(goal_pos, n1);
            world->GetRoadPathFinder().MarkNodeDirty(goal_pos, n2);
        }
    }
}
