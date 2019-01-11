// Copyright (c) 2005 - 2019 Settlers Freaks (sf-team at siedler25.org)
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

#include "ai/beowulf/recurrent/MetalworksManager.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/World.h"

#include "gameData/ToolConsts.h"
#include "gameTypes/SettingsTypes.h"
#include "notifications/ToolNote.h"

namespace beowulf {

MetalworksManager::MetalworksManager(Beowulf* beowulf)
    : RecurrentBase(beowulf)
{
    CheckMetalworksExists();
}

void MetalworksManager::OnRun()
{
    if (CheckMetalworksExists()) {
        if (isWorking_)
            return;

        PlaceNextOrder();
    }
}

bool MetalworksManager::JobOrToolOrQueueSpace(Job job, bool addMetalworksRequest, unsigned maxQueueLength)
{
    for (const Building* building : beowulf_->world.GetBuildings()) {
        if (!building->IsWarehouse())
            continue;
        if (building->GetJobs(job) > 0)
            return true;
        if (building->GetGoods(JOB_CONSTS[job].tool) > 0)
            return true;
    }

    if (metalworksPt_.isValid() && addMetalworksRequest && requests_.size() < maxQueueLength) {
        Request(JOB_CONSTS[job].tool);
        return true;
    }

    return false;
}

void MetalworksManager::OnToolNote(const ToolNote& note)
{
    if (note.type == ToolNote::ToolProduced) {
        requests_.pop();
        if (CheckMetalworksExists())
            PlaceNextOrder();
    }
}

void MetalworksManager::PlaceNextOrder()
{
    if (requests_.empty()) {
        if (isWorking_) {
            beowulf_->GetAII().SetProductionEnabled(metalworksPt_, false);
            isWorking_ = false;
        }
        return;
    }

    GoodType tool = requests_.front();
    PlaceToolOrder(tool, 1);

    if (!isWorking_) {
        beowulf_->GetAII().SetProductionEnabled(metalworksPt_, true);
        isWorking_ = true;
    }
}

bool MetalworksManager::CheckMetalworksExists()
{
    if (metalworksPt_.isValid()) {
        // Still exists?
        Building* building = beowulf_->world.GetBuilding(metalworksPt_);
        if (!building || building->GetType() != BLD_METALWORKS || building->GetState() != Building::Finished) {
            metalworksPt_ = MapPoint::Invalid();

            // If the metalworks was currently working, we have to remove the latest order.
            if (isWorking_)
                PlaceToolOrder(requests_.front(), -1);

            isWorking_ = false;
            return false;
        }
        return true;
    } else {
        // New metalworks exists?
        for (const Building* building : beowulf_->world.GetBuildings(BLD_METALWORKS)) {
            if (building->GetType() == BLD_METALWORKS && building->GetState() == Building::Finished) {
                metalworksPt_ = building->GetPt();
                isWorking_ = false;
                beowulf_->GetAII().SetProductionEnabled(metalworksPt_, false);
                return true;
            }
        }
        return false;
    }
}

void MetalworksManager::PlaceToolOrder(GoodType tool, int8_t count)
{
    static ToolSettings settings;
    static int8_t orders[NUM_TOOLS] = { 0 };
    settings.fill(0);

    unsigned idx = 0;
    for (unsigned i = 0; i < NUM_TOOLS; ++i) {
        if (TOOLS[i] == tool) {
            idx = i;
            break;
        }
    }

    orders[idx] = count;
    beowulf_->GetAII().ChangeTools(settings, orders);
    orders[idx] = 0;
}

}  // namespace beowulf
