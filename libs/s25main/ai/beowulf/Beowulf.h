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
#ifndef BEOWULF_BEOWULF_H_INCLUDED
#define BEOWULF_BEOWULF_H_INCLUDED

#include "ai/beowulf/Resources.h"
#include "ai/beowulf/ProductionPlanner.h"
#include "ai/beowulf/World.h"
#include "ai/beowulf/ExpansionPlanner.h"

#include "ai/AIPlayer.h"

#include <vector>

class BuildingNote;
class ExpeditionNote;
class ResourceNote;
class RoadNote;
class ShipNote;
class NodeNote;
class FlagNote;

namespace beowulf {

class BuildingPlannerBase;

class Beowulf : public AIPlayer
{
    BuildingPlannerBase* buildingPlanner_ = nullptr;
    std::vector<Subscribtion> eventSubscriptions_;
    std::vector<std::pair<Building*, rnet_id_t>> constructionRequests_;
    bool defeated_ = false;

public:
    Resources resources;
    World world;
    ProductionPlanner productionPlanner;

public:
    Beowulf(const unsigned char playerId,
            const GameWorldBase& gwb,
            const AI::Level level);
    ~Beowulf();

    void RunGF(const unsigned gf, bool gfisnwf) override;

    AIInterface& GetAIInterface() { return aii; }
    const AIInterface& GetAIInterface() const { return aii; }

    void RequestConstruction(Building* building, rnet_id_t rnet);

private:
    ExpansionPlanner expansionPlanner_;

    bool CheckDefeat();
    void StartBuildingPlanner();
    void DecommissionUnusedRoads();
    void ResolveGoodsJams();
    void PlaceAdditionalFlags();
    void ConnectRoadNetworks();
    void PlanExpansion();
    void Chat(const std::string& message);

    void OnBuildingNote(const BuildingNote& note);
    void OnExpeditionNote(const ExpeditionNote& note);
    void OnResourceNote(const ResourceNote& note);
    void OnRoadNote(const RoadNote& note);
    void OnShipNote(const ShipNote& note);
    void OnNodeNote(const NodeNote& note);
    void OnFlagNote(const FlagNote& note);
};

} // namespace beowulf

#endif //! BEOWULF_BEOWULF_H_INCLUDED
