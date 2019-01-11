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

#include "ai/beowulf/recurrent/AttackPlanner.h"
#include "ai/beowulf/Beowulf.h"

#include "helpers/containerUtils.h"
#include "buildings/nobMilitary.h"
#include "figures/nofPassiveSoldier.h"
#include "gameData/BuildingConsts.h"

namespace beowulf {

AttackPlanner::AttackPlanner(Beowulf* beowulf)
    : RecurrentBase(beowulf, 50)
{
    // Set military settings.
    MilitarySettings data;
    data[0] = 10;  // Recruiting ratio (to max possible recruits)
    data[1] = 5;   // Send strong soldiers first
    data[2] = 4;   // Active defenders (engaging attackers by leaving building): Chance that one is sent
    data[3] = 5;   // Ratio of used attackers to available attackers
    data[4] = 1;   // Ratio of soldiers in buildings to full occupation for inland
    data[5] = 5;   // Ratio of soldiers in buildings to full occupation for middle region
    data[6] = 5;   // Ratio of soldiers in buildings to full occupation for harbor spots
    data[7] = 8;   // Ratio of soldiers in buildings to full occupation for border region
    beowulf_->GetAII().ChangeMilitary(data);
}

void AttackPlanner::OnRun()
{
    /*
     * - Should we attack all enemies at once, or kill them one by one?
     * - How do we pick an enemy building to attack?
     * - How can we guess how strong an enemy building is?
     *
     *
     * We attack all enemies we can.
     * Attack planning has the goal of destroying the enemy HQ.
     * If that is not attackable, we target harbours.
     * If that is not attackable, we target enemy infrastructure (catapults before other buildings).
     * If chances of destroying enemy infrastructure are low, we target the building with the least expected
     * resistance.
     *
     * How can we estimate resistance?
     * -------------------------------
     * We take the maximum amount of soldiers this building can take.
     *
     * How many soldiers do we sent into attack?
     *
     * @todo: Avoid attacking when the enemy has coins and we don't.
     * @todo: Avoid attacking when our military buildings are waiting to receive coins.
     */

    std::vector<const nobBaseMilitary*> targets = GetPotentialTargets();

    if (targets.empty())
        return;

    // Try to find a HQ and attack that first.
    for (const nobBaseMilitary* target : targets) {
        if (target->GetBuildingType() == BLD_HEADQUARTERS) {
            unsigned attackers = GetAttackersCount(GetAvailableAttackers(target->GetPos()), target->GetPlayer());
            if (attackers > 0) {
                beowulf_->GetAII().Attack(target->GetPos(), attackers, true);
                return;
            }
        }
    }

    // Try to find a harbour to attack.
    for (const nobBaseMilitary* target : targets) {
        if (target->GetBuildingType() == BLD_HARBORBUILDING) {
            unsigned attackers = GetAttackersCount(GetAvailableAttackers(target->GetPos()), target->GetPlayer());
            if (attackers > 0) {
                beowulf_->GetAII().Attack(target->GetPos(), attackers, true);
                return;
            }
        }
    }

    // Try to find building that would destroy enemy catapults or other things upon capturing.
    std::vector<MapPoint> additionalTerritory;
    std::vector<const noBaseBuilding*> destroyed;

    unsigned highestDestruction = 0;
    MapPoint bestTargetPt = MapPoint::Invalid();
    unsigned bestTargetAttackers = 0;

    for (const nobBaseMilitary* target : targets) {
        // Do not attack with less than the power of two most simple soldiers.
        unsigned attackers = GetAttackersCount(GetAvailableAttackers(target->GetPos()), target->GetPlayer());
        if (attackers < 2)
            continue;

        beowulf_->world.PredictExpansionResults(
                    target->GetPos(),
                    target->GetBuildingType(),
                    additionalTerritory,
                    destroyed);

        unsigned destruction = BUILDING_SIZE[target->GetBuildingType()];
        for (const noBaseBuilding* building : destroyed) {
            switch (BUILDING_SIZE[building->GetBuildingType()]) {
            case BQ_HUT:
                destruction += 2;
                break;
            case BQ_HOUSE:
                destruction += 5;
                break;
            case BQ_CASTLE:
                destruction += 10;
                break;
            case BQ_MINE:
                destruction += 10;
                break;
            default: break;
            }
            if (building->GetBuildingType() == BLD_CATAPULT)
                destruction += 50;
        }

        if (destruction > highestDestruction) {
            highestDestruction = destruction;
            bestTargetPt = target->GetPos();
            bestTargetAttackers = attackers;
        }
    }

    if (bestTargetPt.isValid())
        beowulf_->GetAII().Attack(bestTargetPt, bestTargetAttackers, true);
}

std::vector<const nobBaseMilitary*> AttackPlanner::GetPotentialTargets() const
{
    std::vector<const nobBaseMilitary*> ret;

    for (const nobMilitary* building : beowulf_->GetAII().GetMilitaryBuildings()) {
        // Skip buildings that are far away from the front.
        if (building->GetFrontierDistance() == nobMilitary::DIST_FAR)
            continue;

        for (const nobBaseMilitary* target : beowulf_->gwb.LookForMilitaryBuildings(building->GetPos(), 2)) {
            if (helpers::contains(ret, target))
                continue;
            if (target->GetGOT() == GOT_NOB_MILITARY) {
                const nobMilitary* mil = static_cast<const nobMilitary*>(target);
                if (mil->IsNewBuilt() || mil->IsUnderAttack())
                    continue;
            }
            if (!beowulf_->world.IsVisible(target->GetPos()))
                continue;
            if (!beowulf_->GetAII().IsPlayerAttackable(target->GetPlayer()))
                continue;
            if (beowulf_->world.CalcDistance(building->GetPos(), target->GetPos()) >= BASE_ATTACKING_DISTANCE)
                continue;
            ret.push_back(target);
        }
    }

    return ret;
}

std::array<unsigned, 5> AttackPlanner::GetAvailableAttackers(const MapPoint& pt) const
{
    std::array<unsigned, 5> ret;
    ret.fill(0);

    for(const nobBaseMilitary* otherMilBld : beowulf_->gwb.LookForMilitaryBuildings(pt, 2)) {
        if (otherMilBld->GetPlayer() != beowulf_->GetPlayerId())
            continue;

        const nobMilitary* myMil = dynamic_cast<const nobMilitary*>(otherMilBld);
        if (!myMil || myMil->IsUnderAttack())
            continue;

        for (const nofPassiveSoldier* soldier : myMil->GetSoldiersForAttack(pt)) {
            ret[soldier->GetRank()]++;
        }
    }

    return ret;
}

unsigned AttackPlanner::GetAttackersCount(const std::array<unsigned, 5>& soldiers, unsigned char enemy) const
{
//    // Are we producing coins?
//    if (beowulf_->produce.GetTotalProduction(BGD_COIN) > 0) {
//        return soldiers[4]; // only send generals
//    }

//    // Is the enemy producing coins according to statistics?
//    const GamePlayer::Statistic& statistic = beowulf_->gwb.GetPlayer(enemy).GetStatistic(STAT_15M);
//    if (statistic.data[STAT_GOLD][statistic.currentIndex] > 0) {
//        return 0;
//    }

    (void)enemy;

    // All of the best soldiers we have.
    for (int i = 4; i >= 0; --i) {
        if (soldiers[i] > 0)
            return soldiers[i];
    }

    return 0;
}

} // namespace beowulf
