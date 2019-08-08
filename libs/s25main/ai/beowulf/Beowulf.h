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

#include "ai/beowulf/ResourceMap.h"
#include "ai/beowulf/ProductionPlanner.h"
#include "ai/beowulf/Buildings.h"
#include "ai/beowulf/BuildingPlanner.h"

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

/**
 * Todo-List
 * ~~~~~~~~~
 *
 * - How to handle requests that cannot be resolved?
 * - Placing a single building in the building planner does not require
 *   simulated annealing. Simply searching for the best solution is enough.
 */
class Beowulf : public AIPlayer
{
    BuildingPlanner* buildingPlanner_ = nullptr;
    std::vector<Subscribtion> eventSubscriptions_;
    std::vector<std::pair<Building*, rnet_id_t>> constructionRequests_;
    bool defeated_ = false;

public:
    ResourceMap resources;
    Buildings buildings;
    ProductionPlanner productionPlanner;

public:
    Beowulf(const unsigned char playerId, const GameWorldBase& gwb,
            const AI::Level level);
    ~Beowulf();

    void RunGF(const unsigned gf, bool gfisnwf) override;

    AIInterface& GetAIInterface();
    const AIInterface& GetAIInterface() const;

    void RequestConstruction(Building* building, rnet_id_t island);

private:
    bool CheckDefeat();
    void StartBuildingPlanner();
    void DecommissionUnusedRoads();
    void ResolveGoodsJams();
    void PlaceAdditionalFlags();
    void ConnectIslands();
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
