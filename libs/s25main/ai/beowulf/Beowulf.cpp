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

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/BuildingPlannerSimple.h"

#include "network/GameClient.h"
#include "network/GameMessages.h"

#include "notifications/BuildingNote.h"
#include "notifications/RoadNote.h"
#include "notifications/ExpeditionNote.h"
#include "notifications/NodeNote.h"
#include "notifications/ResourceNote.h"
#include "notifications/ShipNote.h"
#include "notifications/FlagNote.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdio>

namespace beowulf {

enum Event {
    PlaceBuildingsEvent = 0,
    PlanRoadsEvent,
    UpdateResoucesForProductinoPlanningEvent,
    PlanProductionEvent,
};

Beowulf::Beowulf(const unsigned char playerId,
                 const GameWorldBase& gwb,
                 const AI::Level level)
    : AIPlayer(playerId, gwb, level),
      resources(aii, 100),
      buildings(aii),
      productionPlanner(*this)
{
    // Initialize event handling.
    NotificationManager& notifications = gwb.GetNotifications();
    eventSubscriptions_.push_back(notifications.subscribe<BuildingNote>(
        boost::lambda::if_(boost::lambda::bind(&BuildingNote::player, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnBuildingNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<ExpeditionNote>(
        boost::lambda::if_(boost::lambda::bind(&ExpeditionNote::player, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnExpeditionNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<ResourceNote>(
        boost::lambda::if_(boost::lambda::bind(&ResourceNote::player, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnResourceNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<RoadNote>(
        boost::lambda::if_(boost::lambda::bind(&RoadNote::player, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnRoadNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<ShipNote>(
        boost::lambda::if_(boost::lambda::bind(&ShipNote::player, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnShipNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<NodeNote>(
        boost::lambda::if_(boost::lambda::bind(&NodeNote::type, boost::lambda::_1) == NodeNote::BQ)
                                      [boost::lambda::bind(&Beowulf::OnNodeNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<FlagNote>(
        boost::lambda::if_(boost::lambda::bind(&FlagNote::type, boost::lambda::_1) == playerId)
                                      [boost::lambda::bind(&Beowulf::OnFlagNote, this, boost::lambda::_1)]));
}

Beowulf::~Beowulf()
{
    if (buildingPlanner_)
        delete buildingPlanner_;
}

void Beowulf::RunGF(const unsigned /*gf*/, bool /*gfisnwf*/)
{
    if (CheckDefeat())
        return;

    /*
     * Kickstart a new building planner.
     */
    if (buildingPlanner_ == nullptr && !constructionRequests_.empty()) {
        std::vector<Building*> requests;
        rnet_id_t rnet = constructionRequests_.front().second;

        for (auto& req : constructionRequests_)
            if (req.second == rnet)
                requests.push_back(req.first);

        constructionRequests_.erase(
                    std::remove_if(
                        constructionRequests_.begin(),
                        constructionRequests_.end(),
                        [rnet](const auto& x) { return x.second == rnet; }),
                    constructionRequests_.end());

        buildingPlanner_ = new BuildingPlannerSimple(aii, buildings, resources, rnet);
        buildingPlanner_->Init(requests);
    }

    if (buildingPlanner_) {
        if (buildingPlanner_->GetSearches() >= buildingPlanner_->GetMaxSearches()) {
            buildingPlanner_->Execute();
            delete buildingPlanner_;
            buildingPlanner_ = nullptr;
        } else {
            for (unsigned i = 0; i < 10 && buildingPlanner_->GetSearches() < buildingPlanner_->GetMaxSearches(); ++i)
                buildingPlanner_->Search();
        }
    }

    DecommissionUnusedRoads();
    ResolveGoodsJams();
    PlaceAdditionalFlags();
    ConnectIslands();

    // PlanProduction
    // PlanStorehouses
    // DistributeGoods
    // PlanExpansion
    // PlanAttack
    // PlanTrading
    // PlanAlliances
}

AIInterface& Beowulf::GetAIInterface()
{
    return aii;
}

const AIInterface& Beowulf::GetAIInterface() const
{
    return aii;
}

void Beowulf::RequestConstruction(Building* building, rnet_id_t island)
{
    constructionRequests_.push_back({ building, island });
}

bool Beowulf::CheckDefeat()
{
    // Check for defeat.
    if (defeated_)
        return true;

    if (aii.GetStorehouses().empty()) {
        aii.Surrender();
        Chat(_("This is the end for me."));
        defeated_ = true;
        return true;
    }

    return false;
}

void Beowulf::DecommissionUnusedRoads()
{
    // @todo: this requires usage analysis of roads.
}

void Beowulf::ResolveGoodsJams()
{

}

void Beowulf::PlaceAdditionalFlags()
{

}

void Beowulf::ConnectIslands()
{

}

void Beowulf::Chat(const std::string& message)
{
    GAMECLIENT.GetMainPlayer().sendMsgAsync(
                new GameMessage_Chat(playerId, CD_ALL, message));
}

void Beowulf::OnBuildingNote(const BuildingNote& note)
{
    Building* bld = buildings.Get(note.pos);

    RTTR_Assert(!bld || bld->GetType() == note.bld);
    RTTR_Assert(note.player == aii.GetPlayerId());

    switch (note.type) {
    case BuildingNote::BuildingSiteAdded:
    {
        buildings.SetState(bld, Building::UnderConstruction);
    } break;

    case BuildingNote::SetBuildingSiteFailed:
    {
        if (buildings.GetFlagState(bld->GetFlag()) == FlagRequested) {
            buildings.RemoveFlag(bld->GetFlag());
        }
        buildings.Remove(bld); // @todo: reset building planner
    } break;

    case BuildingNote::DestructionFailed:
    {
        if (aii.gwb.GetNO(note.pos)->GetType() == NOP_BUILDINGSITE)
            buildings.SetState(bld, Building::UnderConstruction);
        else
            buildings.SetState(bld, Building::Finished);
    } break;

    // The construction of a building finished.
    case BuildingNote::Constructed:
    {
        buildings.SetState(bld, Building::Finished);
    } break;

    // A building was destroyed.
    case BuildingNote::Destroyed:
    {
        buildings.Remove(bld); // @todo: reset building planner
    } break;

    // Military building was captured.
    case BuildingNote::Captured:
    {
        // The first soldier arrived at a military building we built.
        if (bld) {
            RTTR_Assert(bld->GetState() == Building::Finished);
            // noop
        }

        // We captured an enemy military building.
        else {
            bld = buildings.Create(note.bld, Building::Finished);
            buildings.SetPos(bld, note.pos);

//            std::vector<Direction> route;
//            BuildingQualityCalculator bqc(aii);
//            if (FindPath(bld->GetFlag(), bld->GetGoodsDest()->GetFlag(), aii.gwb,
//                // condition
//                [&](const MapPoint& pos, Direction dir)
//                {
//                    return bqc.CanMoveToBuffered(pos, dir);
//                }, &route))
//            {
//                buildings.AddRoadUser(bld, route);
//            }
        }
    } break;

    // A military building was captured by an enemy.
    case BuildingNote::Lost:
    {
        buildings.Remove(bld);// @todo: reset building planner
    } break;

    // Building can't find any more resources.
    case BuildingNote::NoRessources:
    {
        buildings.Deconstruct(bld);
    } break;

    // Lua request to build this building
    case BuildingNote::LuaOrder:
    {
        bld = buildings.Create(note.bld, Building::PlanningRequest, InvalidProductionGroup, note.pos);
        rnet_id_t island = buildings.GetRoadNetwork(buildings.GetGoodsDest(bld, InvalidRoadNetwork, note.pos)->GetFlag()); // Use the goods dest as island.
        RequestConstruction(bld, island);
    } break;

    // Land lost due to enemy building.
    case BuildingNote::LostLand:
    {
        // occurs after OnRoadDestroyed()/OnFlagDestroyed()
        // noop
    } break;
    }
}

void Beowulf::OnExpeditionNote(const ExpeditionNote& note)
{
    (void)note;
}

void Beowulf::OnResourceNote(const ResourceNote& note)
{
    (void)note;
}

void Beowulf::OnRoadNote(const RoadNote& note)
{
    switch (note.type)
    {
    case RoadNote::Constructed:
    {
        buildings.SetRoadState(note.pos, note.route, RoadFinished);
    } break;
    case RoadNote::Destroyed:
    case RoadNote::ConstructionFailed:
    {
        buildings.RemoveRoad(note.pos, note.route); // @todo: reset building planner
        // recalculate road users
    } break;
    }
}

void Beowulf::OnShipNote(const ShipNote& note)
{
    (void)(note);
}

void Beowulf::OnNodeNote(const NodeNote& note)
{
    (void)(note);
    /*
     * In case of BQ degradation, we could update a running planner, reducing
     * its size. But then again the planner could verify its best solutions with
     * the current world state and we can save some implementation complexity.
     */
}

void Beowulf::OnFlagNote(const FlagNote& note)
{
    switch (note.type)
    {
    case FlagNote::Constructed:
    {
        buildings.SetFlagState(note.pt, FlagFinished);
    } break;

    case FlagNote::DestructionFailed:
    {
        buildings.SetFlagState(note.pt, FlagFinished);
    } break;

    case FlagNote::ConstructionFailed:
    case FlagNote::Captured:
    case FlagNote::Destroyed:
    {
        buildings.RemoveFlag(note.pt);
    } break;
    }
}

} // namespace beowulf
