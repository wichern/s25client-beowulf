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

#include "ai/beowulf/recurrent/CoinManager.h"
#include "ai/beowulf/Beowulf.h"

#include "notifications/BuildingNote.h"
#include "buildings/nobMilitary.h"
#include "figures/nofPassiveSoldier.h"
#include "GlobalGameSettings.h"

namespace beowulf {

CoinManager::CoinManager(Beowulf* beowulf)
    : RecurrentBase(beowulf, 10)
{

}

void CoinManager::OnRun()
{
    // Do nothing until we have coins.
    if (beowulf_->produce.GetTotalProduction(BGD_COIN) == 0)
        return;

    // Place a fortress in the HQ region for spending coins.
    if (!academy_) {
        RequestAcademy();
        return;
    } else if (0 == beowulf_->build.GetRequestCount({ BLD_FORTRESS }, beowulf_->world.GetHQFlag())) {
        // If building failed we have to try again.
        RequestAcademy();
        return;
    }

    if (academy_->GetState() != Building::Finished)
        return;

    const nobMilitary* acad = beowulf_->GetAII().gwb.GetSpecObj<nobMilitary>(academy_->GetPt());
    RTTR_Assert(acad);

    // Enable/Disable coins for academy based on the number of soldiers that can be trained.
    const unsigned maxRank = beowulf_->GetAII().gwb.GetGGS().GetMaxMilitaryRank();
    unsigned soldiers = 0;
    for(const nofPassiveSoldier* soldier : acad->GetTroops())
        if(soldier->GetRank() < maxRank)
            soldiers++;

    if (soldiers < 3) {
        if (!acad->IsGoldDisabled())
            beowulf_->GetAII().SetCoinsAllowed(academy_->GetPt(), false);
    } else {
        if (acad->IsGoldDisabled())
            beowulf_->GetAII().SetCoinsAllowed(academy_->GetPt(), true);
    }

    // Send generals away.
    if (acad->HasMaxRankSoldier())
        beowulf_->GetAII().SendSoldiersHome(academy_->GetPt());
    if (acad->GetTroops().size() < acad->GetMaxTroopsCt())
        beowulf_->GetAII().OrderNewSoldiers(academy_->GetPt());
}

void CoinManager::OnBuildingNote(const BuildingNote& note)
{
    switch (note.type) {
    case BuildingNote::SetBuildingSiteFailed:
    {
        academy_ = nullptr;
    } break;

    // The construction of a building finished.
    case BuildingNote::Captured:
    case BuildingNote::Constructed:
    {
        // Disable coins on every position.
        const nobMilitary* mil = beowulf_->GetAII().gwb.GetSpecObj<nobMilitary>(note.pos);
        if (mil && !mil->IsGoldDisabled())
            beowulf_->GetAII().SetCoinsAllowed(note.pos, false);
    } break;

    // A building was destroyed.
    case BuildingNote::Destroyed:
    {
        if (academy_ && note.pos == academy_->GetPt()) {
            academy_ = nullptr;
        }
    } break;

    default:
        // noop
        break;
    }
}

void CoinManager::RequestAcademy()
{
    MapPoint regionPt = beowulf_->world.GetHQFlag();

    BuildLocations locations(beowulf_->world, false);
    locations.Calculate(regionPt);

    unsigned lowestDistance = std::numeric_limits<unsigned>::max();
    MapPoint bestLocation = MapPoint::Invalid();
    for (const MapPoint& loc : locations.Get()) {
        unsigned distance = beowulf_->world.CalcDistance(loc, regionPt);
        if (distance < lowestDistance) {
            lowestDistance = distance;
            bestLocation = loc;
        }
    }

    if (bestLocation.isValid()) {
        if (!academy_) {
            academy_ = beowulf_->world.Create(
                        BLD_FORTRESS,
                        Building::ConstructionRequested,
                        InvalidProductionGroup,
                        bestLocation);
        } else {
            beowulf_->world.SetPoint(academy_, bestLocation);
        }
        beowulf_->build.Request(academy_, regionPt);
    }
}

} // namespace beowulf
