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

#include "ai/beowulf/World.h"
#include "ai/beowulf/recurrent/RoadManager.h"
#include "ai/beowulf/recurrent/BuildingPlanner.h"
#include "ai/beowulf/recurrent/ExpansionPlanner.h"
#include "ai/beowulf/recurrent/ProductionPlanner.h"
#include "ai/beowulf/recurrent/AttackPlanner.h"
#include "ai/beowulf/recurrent/MetalworksManager.h"
#include "ai/beowulf/recurrent/CoinManager.h"

#include "ai/AIPlayer.h"

#include <vector>
#include <ctime>

namespace beowulf {

class RecurrentBase;

class Beowulf : public AIPlayer
{
public:
    World world;
    BuildingPlanner build;
    RoadManager roads;
    ExpansionPlanner expand;
    ProductionPlanner produce;
    MetalworksManager metalworks;
    AttackPlanner attack;
    CoinManager coins;

public:
    Beowulf(const unsigned char playerId,
            const GameWorldBase& gwb,
            const AI::Level level);
    ~Beowulf() override;

    void RunGF(const unsigned gf, bool gfisnwf) override;

    void DisableRecurrents();

    AIInterface& GetAII() { return aii; }
    const AIInterface& GetAII() const { return aii; }

    void ClearWorstRuntime();
    const std::vector<std::clock_t>& GetWorstRuntime() const { return recurrentsWorstRuntime_; }

private:
    void Chat(const std::string& message) const;
    bool CheckDefeat();

private:
    bool defeated_ = false;
    bool waitForNextSync_ = false;

    std::vector<std::clock_t> recurrentsWorstRuntime_;

    std::vector<RecurrentBase*> recurrents_;
};

} // namespace beowulf

#endif //! BEOWULF_BEOWULF_H_INCLUDED
