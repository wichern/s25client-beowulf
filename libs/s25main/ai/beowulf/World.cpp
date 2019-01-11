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

#include "ai/beowulf/World.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Building.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "buildings/noBuildingSite.h"
#include "buildings/nobUsual.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobBaseWarehouse.h"
#include "buildings/nobHQ.h"
#include "gameData/BuildingProperties.h"
#include "gameData/GameConsts.h"
#include "gameData/MilitaryConsts.h"
#include "nodeObjs/noFlag.h"
#include "pathfinding/PathConditionRoad.h"
#include "GlobalGameSettings.h"
#include "addons/const_addons.h"
#include "notifications/BuildingNote.h"
#include "notifications/RoadNote.h"
#include "notifications/FlagNote.h"
#include "helpers/containerUtils.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>

#include <limits>
#include <set>

namespace beowulf {

World::World(Beowulf* beowulf, bool fow)
    : resources(beowulf->GetAII(), *this, fow),
      beowulf_(beowulf),
      aii_(beowulf->GetAII()),
      fow_(fow)
{
    Resize(aii_.gwb.GetSize());
    nodes_.Resize(GetSize());

    // Set existing buildings.
    const nobHQ* bld = aii_.GetHeadquarter();
    if (bld) {
        SetPoint(Create(BLD_HEADQUARTERS, Building::Finished), bld->GetPos());
        hqFlag_ = GetNeighbour(bld->GetPos(), Direction::SOUTHEAST);
    } else {
        hqFlag_ = MapPoint::Invalid();
    }

    for (const noBuildingSite* bld : aii_.GetBuildingSites())
        SetPoint(Create(bld->GetBuildingType(), Building::UnderConstruction), bld->GetPos());

    for (const nobMilitary* bld : aii_.GetMilitaryBuildings())
        SetPoint(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    for (unsigned i = FIRST_USUAL_BUILDING; i < NUM_BUILDING_TYPES; ++i)
        for (const nobUsual* bld : aii_.GetBuildings(static_cast<BuildingType>(i)))
            SetPoint(Create(bld->GetBuildingType(), Building::Finished), bld->GetPos());

    // Set existing roads.
    RTTR_FOREACH_PT(MapPoint, GetSize()) {
        Node& node = nodes_[pt];
        const noFlag* flagObj = aii_.gwb.GetSpecObj<noFlag>(pt);
        if (flagObj && flagObj->GetPlayer() == aii_.GetPlayerId())
            SetFlagState(pt, FlagFinished);
        for (unsigned char rdir = 0; rdir < 3; ++rdir) {
            if (aii_.gwb.GetRoad(pt, rdir)) {
                if (aii_.gwb.GetNO(pt)->GetType() != NOP_BUILDING &&
                        aii_.gwb.GetNO(GetNeighbour(pt, OppositeDirection(Direction(rdir))))->GetType() != NOP_BUILDING)
                {
                    node.roads[rdir] = RoadFinished;
                }
            }
        }
    }

    // Initialize event handling.
    NotificationManager& notifications = aii_.gwb.GetNotifications();
    eventSubscriptions_.push_back(notifications.subscribe<BuildingNote>(
        boost::lambda::if_(boost::lambda::bind(&BuildingNote::player, boost::lambda::_1) == aii_.GetPlayerId())
                                      [boost::lambda::bind(&World::OnBuildingNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<RoadNote>(
        boost::lambda::if_(boost::lambda::bind(&RoadNote::player, boost::lambda::_1) == aii_.GetPlayerId())
                                      [boost::lambda::bind(&World::OnRoadNote, this, boost::lambda::_1)]));
    eventSubscriptions_.push_back(notifications.subscribe<FlagNote>(
        boost::lambda::if_(boost::lambda::bind(&FlagNote::player, boost::lambda::_1) == aii_.GetPlayerId())
                                      [boost::lambda::bind(&World::OnFlagNote, this, boost::lambda::_1)]));
}

World::~World()
{
    for (Building* building : buildings_)
        delete building;
}

void World::Construct(Building* building, const MapPoint& pt)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::Finished);
    RTTR_Assert(building->GetState() != Building::ConstructionRequested);
    RTTR_Assert(building->GetState() != Building::UnderConstruction);
    RTTR_Assert(!IsRestricted(pt));

    ClearPlan();

    aii_.SetBuildingSite(pt, building->GetType());

    building->state_ = Building::ConstructionRequested;
    SetPoint(building, pt);
    if (!HasFlag(building->GetFlag()))
        SetFlagState(building->GetFlag(), FlagRequested);
}

void World::ConstructFlag(const MapPoint& pt)
{
    ClearPlan();

    Node& node = nodes_[pt];
    if (node.flag == FlagDoesNotExist) {
        aii_.SetFlag(pt);
        SetFlagState(pt, FlagRequested);
    }
}

void World::ConstructRoad(const MapPoint& pt, const std::vector<Direction>& route)
{
    RTTR_Assert(route.size() >= 2);
    ClearPlan();

    const Node& node = nodes_[pt];

    if (!(node.flag == FlagRequested || node.flag == FlagFinished)) {
        aii_.SetFlag(pt);
        SetFlagState(pt, FlagRequested);
    }
    aii_.BuildRoad(pt, false, route);
    SetRoadState(pt, route, RoadRequested);

    MapPoint end = pt;
    for (Direction dir : route)
        end = GetNeighbour(end, dir);
    if (!HasFlag(end))
        SetFlagState(end, FlagRequested);
}

void World::Deconstruct(Building* building)
{
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() != Building::PlanningRequest);

    ClearPlan();

    aii_.DestroyBuilding(building->GetPt());
    building->state_ = Building::DestructionRequested;
}

void World::DeconstructFlag(const MapPoint& pt)
{
    ClearPlan();

    Node& node = nodes_[pt];
    RTTR_Assert(node.flag == FlagFinished);

    const noFlag* nof = aii_.gwb.GetSpecObj<noFlag>(pt);
    RTTR_Assert(nof);
    aii_.DestroyFlag(nof);

    // Mark all connected roads as deconstruction requested as well.
    FloodFill(*this, pt,
    // condition
    [&](const MapPoint& pt2, Direction dir)
    {
        if (HasRoad(pt2, dir)) {
            SetRoadState(pt2, dir, RoadDestructionRequested);
            return !HasFlag(GetNeighbour(pt2, dir));
        } else
            return false;
    },
    // action
    [&](const MapPoint&) {});

    SetFlagState(pt, FlagDestructionRequested);
}

void World::DeconstructRoad(const MapPoint& pt, const std::vector<Direction>& route)
{
    RTTR_Assert(nodes_[pt].flag == FlagRequested || nodes_[pt].flag == FlagFinished);
    RTTR_Assert(!route.empty());
    RTTR_Assert(HasRoad(pt, route.front()));

    ClearPlan();

    aii_.DestroyRoad(pt, route.front());
    SetRoadState(pt, route, RoadDestructionRequested);
}

void World::Plan(Building* building, const MapPoint& pt)
{
    Node& node = nodes_[pt];
    RTTR_Assert(node.building == nullptr);
    RTTR_Assert(building);
    RTTR_Assert(building->GetState() == Building::PlanningRequest);

    node.building = building;
    building->pt_ = pt;

    resources.Added(pt, building->GetType());

    PlanFlag(GetNeighbour(pt, Direction::SOUTHEAST));

    activePlan_ = true;
}

void World::PlanFlag(const MapPoint& pt)
{
    Node& node = nodes_[pt];

    if (node.flag == FlagFinished || node.flag == FlagRequested)
        return;

    node.flagPlanCount++;

    if (node.flagPlanCount == 1)
        flags_.push_back(pt);

    activePlan_ = true;
}

void World::PlanRoad(
        const MapPoint& pt,
        const std::vector<Direction>& route)
{
    MapPoint cur = pt;
    for (size_t i = 0; i < route.size(); ++i) {
        PlanSegment(cur, route[i]);
        cur = GetNeighbour(cur, route[i]);
    }

    PlanFlag(cur);

    activePlan_ = true;
}

void World::ClearPlan()
{
    if (!activePlan_)
        return;

    // Clear planned flags.
    flags_.erase(std::remove_if(flags_.begin(), flags_.end(),
        [&](const MapPoint& pt) { return nodes_[pt].flagPlanCount > 0; }), flags_.end());

    // Clear all nodes
    RTTR_FOREACH_PT(MapPoint, GetSize())
    {
        Node& node = nodes_[pt];

        if (node.building && node.building->GetState() == Building::PlanningRequest) {
            resources.Removed(pt, node.building->GetType());
            node.building = nullptr;
        }

        node.flagPlanCount = 0;
        node.roadPlanCount[0] = 0;
        node.roadPlanCount[1] = 0;
        node.roadPlanCount[2] = 0;
    }

    activePlan_ = false;
}

void World::Remove(Building* building)
{
    ClearPlan();

    if (building->GetGroup() != InvalidProductionGroup) {
        ProductionGroup& group = groups[building->GetGroup()];
        for (size_t i = 0; i < group.types.size(); ++i)
            if (group.buildings[i] == building)
                group.buildings[i] = nullptr;
    }

    if (building->GetPt().isValid())
        nodes_[building->GetPt()].building = nullptr;

    resources.Removed(building->GetPt(), building->GetType());

    buildings_.erase(std::remove(buildings_.begin(), buildings_.end(), building), buildings_.end());

    delete building;
}

void World::RemoveFlag(const MapPoint& pt)
{
    ClearPlan();
    SetFlagState(pt, FlagDoesNotExist);
}

void World::RemoveRoad(
        const MapPoint& pt,
        const std::vector<Direction>& route)
{
    ClearPlan();
    SetRoadState(pt, route, RoadDoesNotExist);
}

Building* World::Create(
        BuildingType type,
        Building::State state,
        unsigned group,
        const MapPoint& pt)
{
    ClearPlan();

    Building* building = new Building(*this, type, state);
    building->pt_ = pt;
    buildings_.push_back(building);

    // If the building already comes with a desired destination group we respect that.
    if (group != InvalidProductionGroup) {
        building->group_ = group;
        bool inserted = false;
        for (size_t i = 0; i < groups[group].types.size(); ++i) {
            if (groups[group].types[i] == type && nullptr == groups[group].buildings[i]) {
                groups[group].buildings[i] = building;
                inserted = true;
                break;
            }
        }
        RTTR_Assert(inserted);
    }

    return building;
}

std::vector<Building*> World::GetBuildings(BuildingType type) const
{
    std::vector<Building*> ret;
    for (Building* building : GetBuildings()) {
        if (building->GetType() == type)
            ret.push_back(building);
    }
    return ret;
}

Building* World::GetBuilding(const MapPoint& pt) const
{
    if (pt.isValid())
        return nodes_[pt].building;
    return nullptr;
}

Building* World::GetHQ() const
{
    const nobHQ* hq = aii_.GetHeadquarter();
    if (hq)
        return nodes_[hq->GetPos()].building;
    return nullptr;
}

bool World::HasBuilding(const MapPoint& pt) const
{
    return GetBuilding(pt) != nullptr;
}

void World::SetState(Building* building, Building::State state)
{
    building->state_ = state;
}

bool World::HasFlag(const MapPoint& pt) const
{
    const Node& node = nodes_[pt];
    return node.flag == FlagRequested || node.flag == FlagFinished || node.flagPlanCount > 0;
}

FlagState World::GetFlagState(const MapPoint& pt) const
{
    return nodes_[pt].flag;
}

void World::SetFlagState(const MapPoint& pt, FlagState state)
{
    RTTR_Assert(nodes_[pt].flag != state);

    // new flag?
    if (!HasFlag(pt) && (state == FlagFinished || state == FlagRequested)) {
        flags_.push_back(pt);
    }

    // removed flag?
    if (HasFlag(pt) && (state == FlagDestructionRequested || state == FlagDoesNotExist)) {
        flags_.erase(std::find(flags_.begin(), flags_.end(), pt), flags_.end());
    }

    nodes_[pt].flag = state;
}

bool World::IsPointConnected(const MapPoint& pt) const
{
    for (Direction dir : Direction()) {
        if (HasRoad(pt, dir))
            return true;
    }

    return false;
}

bool World::IsBorder(const MapPoint& pt, bool includeAnticipated) const
{
    return IsOwner(pt, includeAnticipated) && !IsPlayerTerritory(pt, includeAnticipated);
}

bool World::IsPlayerTerritory(const MapPoint& pt, bool includeAnticipated) const
{
    bool owned = IsOwner(pt, includeAnticipated);
    if (!owned)
        return false;

    // Neighbour nodes must belong to this player
    for (Direction dir : Direction()) {
        if (!IsOwner(GetNeighbour(pt, dir), includeAnticipated))
            return false;
    }

    return true;
}

bool World::HasRoad(const MapPoint& pt, Direction dir) const
{
    MapPoint nodePt;
    unsigned idx;

    if (dir.toUInt() >= 3) {
        nodePt = pt;
        idx = OppositeDirection(dir).toUInt();
    } else {
        nodePt = GetNeighbour(pt, dir);
        idx = dir.toUInt();
    }

    const Node& node = nodes_[nodePt];
    RoadState state = node.roads[idx];
    return state == RoadRequested || state == RoadFinished || node.roadPlanCount[idx] > 0;
}

bool World::IsOnRoad(const MapPoint& pt) const
{
    for (Direction dir : Direction()) {
        if (HasRoad(pt, dir))
            return true;
    }
    return false;
}

RoadState World::GetRoadState(const MapPoint& pt, Direction dir) const
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        return nodes_[pt].roads[oppositeDir.toUInt()];
    else
        return nodes_[GetNeighbour(pt, dir)].roads[dir.toUInt()];
}

void World::SetRoadState(const MapPoint& pt, Direction dir, RoadState state)
{
    Direction oppositeDir = OppositeDirection(dir);
    if (dir.toUInt() >= 3)
        nodes_[pt].roads[oppositeDir.toUInt()] = state;
    else
        nodes_[GetNeighbour(pt, dir)].roads[dir.toUInt()] = state;
}

void World::SetRoadState(
        const MapPoint& pt,
        const std::vector<Direction>& route,
        RoadState state)
{
    MapPoint cur = pt;
    for (size_t i = 0; i < route.size(); ++i) {
        SetRoadState(cur, route[i], state);
        cur = GetNeighbour(cur, route[i]);
    }
}

BuildingQuality World::GetBQ(
        const MapPoint& pt,
        bool includeAnticipated,
        const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps) const
{
    BQCalculator2 bqc(*this, beowulf_->GetAII().gwb, tmps);
    BuildingQuality bq = bqc(pt, [this, pt](const MapPoint& pt2){
        for (Direction dir : Direction())
            if (HasRoad(pt2, dir))
                return true;
        return false;
    });
    return AdjustBQ(pt, bq, includeAnticipated);
}

bool World::IsRoadPossible(
        const MapPoint& pt,
        Direction dir,
        bool includeAnticipated,
        const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps) const
{
    MapPoint dest = GetNeighbour(pt, dir);

    // If 'pt' already has more than one road and we can't place a flag a road is impossible.
    if (!HasFlag(pt)) {
        bool hasRoads = false;
        for (Direction d : Direction()) {
            if (HasRoad(pt, d)) {
                hasRoads = true;
                break;
            }
        }
        if (hasRoads && GetBQ(pt, includeAnticipated, tmps) < BQ_FLAG)
            return false;
    }

    if (HasFlag(dest)) {
        // Do we need to place a flag at pt?
        unsigned roads = 0;
        for (Direction d2 : Direction()) {
            if (HasRoad(pt, d2))
                roads++;
        }
        if (roads >= 2)
            return false;

        return true;
    }

    // If dest already has roads, we need to check whether we can place a flag
    // and not need to place a flag from where we come.
    for (Direction d : Direction()) {
        if (HasRoad(dest, d)) {
            // Can we place a flag at dest?
            if (GetBQ(dest, includeAnticipated, tmps) < BQ_FLAG)
                return false;

            // Do we need to place a flag at pt?
            unsigned roads = 0;
            for (Direction d2 : Direction()) {
                if (HasRoad(pt, d2))
                    roads++;
            }
            if (roads >= 2)
                return false;

            return true;
        }
    }

    if (GetBM(dest, tmps) != BlockingManner::None)
        return false;

    if (!IsPlayerTerritory(dest, includeAnticipated))
        return false;

    return aii_.gwb.IsRoadAvailable(false, dest);
}

std::vector<Direction> World::GetPath(
        const MapPoint& src,
        const MapPoint& dst) const
{
    std::vector<Direction> ret;
    FindPath(src, *this, &ret,
        [this](const MapPoint &pt, Direction dir) { return HasRoad(pt, dir); }, // Condition
        [&dst](const MapPoint& pt) { return pt == dst; }, // End
        [](const MapPoint&) { return 1; }, // Heuristic
        [](const MapPoint&, Direction) { return 1; });
    return ret;
}

std::pair<MapPoint, unsigned> World::GetNearestBuilding(
        const MapPoint& pt,
        const std::vector<BuildingType>& types,
        const Building* self) const
{
    // shortest euclidean distance.
    unsigned dist = std::numeric_limits<unsigned>::max();
    MapPoint closest = MapPoint::Invalid();

    uint64_t query = 0;
    for (BuildingType type : types)
        query |= 1ul << static_cast<uint64_t>(type);

    for (const Building* building : GetBuildings()) {
        // Skip buildings of different type.
        if (0 == (query & (1ul << static_cast<uint64_t>(building->GetType()))))
            continue;
        // Skip buildings that have no position yet.
        if (!building->GetPt().isValid())
            continue;
        if (building == self)
            continue;

        unsigned d = CalcDistance(pt, building->GetPt());
        if (d < dist) {
            closest = building->GetPt();
            dist = d;
        }
    }

    return { closest, dist };
}

bool World::CanBuildMilitary(const MapPoint& pt) const
{
    if (aii_.gwb.IsMilitaryBuildingNearNode(pt, aii_.GetPlayerId()))
        return false;
    for (const Building* bld : buildings_)
        if (BuildingProperties::IsMilitary(bld->GetType()))
            if (CalcDistance(bld->GetPt(), pt) <= 4)
                return false;
    return true;
}

Building* World::GetGoodsDest(
        const Building* building,
        const MapPoint& pt) const
{
    // Get destination infos for this type of building.
    bool checkGroup;
    const std::vector<BuildingType>& types = building->GetDestTypes(checkGroup);

    if (checkGroup) {
        for (Building* bld : GetBuildings()) {
            // Is the building in the same group?
            if (bld->GetGroup() != building->GetGroup())
                continue;

            // Does the building have the same type?
            if (helpers::contains(types, bld->GetType()))
                return bld;
        }
    }

    auto nearest = GetNearestBuilding(pt, types, building);

    return nearest.first.isValid() ? GetBuilding(nearest.first) : nullptr;
}

const noBaseBuilding* World::GetBaseBuilding(const Building* building) const
{
    if (!building)
        return nullptr;

    const noBase* node = aii_.gwb.GetNode(building->GetPt()).obj;
    if (!node)
        return nullptr;

    if (!(node->GetType() == NOP_BUILDING || node->GetType() == NOP_BUILDINGSITE))
        return nullptr;

    return static_cast<const noBaseBuilding*>(node);
}

World::ProductionGroup& World::CreateGroup(
        const std::vector<BuildingType>&& types,
        const MapPoint& region)
{
    unsigned id = static_cast<unsigned>(groups.size());
    groups.push_back({ std::move(types), {}, id, region });
    groups.back().buildings.resize(types.size());
    return groups.back();
}

bool World::IsRestricted(const MapPoint& pt) const
{
    const auto& restrictions = aii_.gwb.GetPlayer(aii_.GetPlayerId()).GetRestrictedArea();
    return helpers::contains(restrictions, pt);
}

bool World::IsVisible(const MapPoint& pt) const
{
    if (!fow_)
        return true;
    return aii_.IsVisible(pt);
}

bool World::CanConnectBuilding(
        const MapPoint& buildingFlag,
        const MapPoint& dst,
        bool includeAnticipated,
        const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps) const
{
    if (buildingFlag == dst)
        return true;

    return FindPath(buildingFlag, *this, nullptr,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
//        if (pt == buildingFlag && dir == Direction::NORTHWEST)
//            return false;
        if (HasRoad(pt, dir))
            return true;
        if (IsRoadPossible(pt, dir, includeAnticipated, tmps))
            return true;
        return false;
    },
    // End
    [&](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == dst;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return CalcDistance(pt, dst);
    },
    // Cost
    [&](const MapPoint&, Direction)
    {
        return 1;
    });
}

bool World::IsConnected(const MapPoint& src, const MapPoint& dst) const
{
    return FindPath(src, *this, nullptr,
    // Condition
    [&](const MapPoint& pt, Direction dir)
    {
        return HasRoad(pt, dir);
    },
    // End
    [&](const MapPoint& pt)
    {
        // The search can end if we found a way to any flag of the destination
        // road network.
        return pt == dst;
    },
    // Heuristic
    [&](const MapPoint& pt)
    {
        return CalcDistance(pt, dst);
    },
    // Cost
    [&](const MapPoint&, Direction)
    {
        return 1;
    });
}

void World::SetCaptured(Building* building)
{
    building->captured_ = true;
}

std::vector<const noBaseBuilding*> World::GetEnemyCatapultsInReach(const MapPoint& pt) const
{
    std::vector<const noBaseBuilding*> ret;

    if (!aii_.gwb.GetGGS().isEnabled(AddonId::LIMIT_CATAPULTS)) //-V807
        return ret;

    VisitPointsInRadius(pt, CATAPULT_ATTACK_RANGE, [&](const MapPoint& p)
    {
        const noBase* obj = aii_.gwb.GetNO(p);
        if (!(obj->GetType() == NOP_BUILDING || obj->GetType() == NOP_BUILDINGSITE))
            return;

        BuildingType buildingType = static_cast<const noBaseBuilding*>(obj)->GetBuildingType();
        if (buildingType != BLD_CATAPULT)
            return;

        if (aii_.IsPlayerAttackable(aii_.gwb.GetNode(p).owner))
            ret.push_back(static_cast<const noBaseBuilding*>(obj));
    }, false);

    return ret;
}

unsigned World::GetEnemySoldiersInReach(const MapPoint& pt) const
{
    unsigned ret = 0;

    sortedMilitaryBlds blds = aii_.gwb.LookForMilitaryBuildings(pt, 2);
    for (const nobBaseMilitary* bld : blds) {
        if (bld->GetGOT() != GOT_NOB_MILITARY)
            continue;
        if (!aii_.IsPlayerAttackable(bld->GetPlayer()))
            continue;
        if (fow_ && !aii_.IsVisible(bld->GetPos()))
            continue;
        if (CalcDistance(pt, bld->GetPos()) >= BASE_ATTACKING_DISTANCE)
            continue;

        switch (bld->GetBuildingType()) {
        case BLD_BARRACKS:
            ret += 2 - 1;
            break;
        case BLD_GUARDHOUSE:
            ret += 3 - 1;
            break;
        case BLD_WATCHTOWER:
            ret += 6 - 1;
            break;
        case BLD_FORTRESS:
            ret += 9 - 1;
            break;
        default: RTTR_Assert(false);
        }
    }

    return ret;
}

bool World::IsEnemyNear(const MapPoint& pt, unsigned radius) const
{
    return CheckPointsInRadius(pt, radius, [&](const MapPoint& p, unsigned)
    {
        if (fow_)
            if (!aii_.IsVisible(p))
                return false;

        unsigned char owner = aii_.gwb.GetNode(p).owner;
        if (owner > 0 && owner != (aii_.GetPlayerId() + 1))
            return true;
        return false;
    }, false);
}

void World::PredictExpansionResults(
        const MapPoint& pt,
        BuildingType type,
        std::vector<MapPoint>& additionalTerritory,
        std::vector<const noBaseBuilding*>& destroyed) const
{
    /*
     * enemies = all military buildings of other players close by.
     * additionalOwned = all points we are now closest to.
     * additionalTerritory = all points we now own (or did own) that is not a border.
     */

    additionalTerritory.clear();
    destroyed.clear();

    unsigned radius = GetMilitaryRadius(type);
    if (0 == radius)
        return;

    // We assume that a military building can be placed here.

    // Get enemy military buildings in range.
    // (Largest known military radius + radius of the new building.)
    std::vector<const nobBaseMilitary*> enemies;
    unsigned short enemySearchRadius = static_cast<unsigned short>(HQ_RADIUS + radius);
    for (const nobBaseMilitary* mb : aii_.gwb.LookForMilitaryBuildings(pt, enemySearchRadius)) {
        if (mb->GetPlayer() != aii_.GetPlayerId() && mb->GetPos() != pt)
            enemies.push_back(mb);
    }

    struct Prediction {
        bool alreadyTerritory = false;
        bool owned = false;
        bool additionallyOwned = false;
        bool destructionChecked = false;
    };
    std::map<MapPoint, Prediction, MapPointComp> predictions;
    std::set<const noBaseBuilding*> destroyedSet;

    // Visit every point in military radius + 2 and check for new ownership and existing
    // enemy buildings.
    VisitPointsInRadius(pt, radius + 2, [&](const MapPoint& p)
    {
        if (IsRestricted(p))
            return;

        Prediction& prediction = predictions[p];

        // Already player territory or about to become?
        if (IsOwner(p, true)) {
            prediction.owned = true;
            if (IsPlayerTerritory(p, true))
                prediction.alreadyTerritory = true;
            return;
        }

        unsigned distance = CalcDistance(pt, p);
        if (distance <= radius) {
            if (IsCloserThanEnemy(p, distance, enemies)) {
                prediction.owned = true;
                prediction.additionallyOwned = true;
            }
        }

        // Destroy all buildings around a point where the owner changed.
        if (!prediction.additionallyOwned)
            return;
        if (aii_.gwb.GetNode(p).owner == aii_.GetPlayerId() + 1)
            return;

        VisitPointsInRadius(p, 1, [&](const MapPoint& destroyPt)
        {
            if (!predictions[destroyPt].destructionChecked) {
                predictions[destroyPt].destructionChecked = true;

                const noBaseBuilding* bld = GetEnemyBuilding(destroyPt);
                if (bld && bld->GetPos() != pt)
                    destroyedSet.insert(bld);
            }
        }, true);
    }, true);

    // Filter all border points to create additionalTerritory.
    for (const auto& prediction : predictions) {
        if (prediction.second.alreadyTerritory)
            continue;

        const MapPoint& p = prediction.first;

        bool isBorder = false;
        for (Direction dir : Direction()) {
            MapPoint neighbour = GetNeighbour(p, dir);
            if (!predictions[neighbour].owned) {
                isBorder = true;
                break;
            }
        }
        if (!isBorder)
            additionalTerritory.push_back(p);
    }

    std::copy(destroyedSet.begin(), destroyedSet.end(), std::back_inserter(destroyed));
}

void World::PlanSegment(const MapPoint& pt, Direction dir)
{
    if (dir.toUInt() >= 3)
        nodes_[pt].roadPlanCount[OppositeDirection(dir).toUInt()]++;
    else
        nodes_[GetNeighbour(pt, dir)].roadPlanCount[dir.toUInt()]++;
}

void World::SetPoint(Building* building, const MapPoint& pt)
{
    Node& node = nodes_[pt];
    RTTR_Assert(node.building == nullptr);

    if (building->pt_.isValid())
        nodes_[building->pt_].building = nullptr;

    node.building = building;
    building->pt_ = pt;

    resources.Added(pt, building->GetType());
}

BlockingManner World::GetBM(
        const MapPoint& pt,
        const std::vector<std::pair<MapPoint, BuildingQuality>>& tmps) const
{
    const Node& node = nodes_[pt];
    if (node.building)
        return BlockingManner::Building;

    if (HasFlag(pt))
        return BlockingManner::Flag;

    // Check for castle extensions
    for (unsigned dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
        Building* building = nodes_[GetNeighbour(pt, Direction(dir))].building;
        if (building && building->GetQuality() == BQ_CASTLE) {
            return BlockingManner::Single;
        }
    }

    for (const auto& bld : tmps) {
        if (bld.first == pt) {
            if (BQ_FLAG == bld.second)
                return BlockingManner::Flag;
            return BlockingManner::Building;
        }

        // Check for castle extensions
        if (BQ_CASTLE == bld.second) {
            if (CalcDistance(pt, bld.first) == 1) {
                for (unsigned dir = Direction::EAST; dir < Direction::COUNT; ++dir) {
                    if (bld.first == GetNeighbour(pt, Direction(dir))) {
                        return BlockingManner::Single;
                    }
                }
            }
        }
    }

    return BlockingManner::None;
}

bool World::IsOwner(const MapPoint& pt, bool includeAnticipated) const
{
    // Already owned without planned buildings?
    if (aii_.gwb.GetNode(pt).owner == aii_.GetPlayerId() + 1)
        return true;

    if (!includeAnticipated)
        return false;

    // @performance: Buffer pressure of our buildings in nodes_.

    // Calculate distance of our buildings to the point.
    unsigned ourDistance = std::numeric_limits<unsigned>::max();

    for (const Building* bld : buildings_) {
        if (!bld->GetPt().isValid())
            continue;
        unsigned distance = CalcDistance(bld->GetPt(), pt);
        if (distance > GetMilitaryRadius(bld->GetType()))
            continue;
        ourDistance = std::min(distance, ourDistance);
    }

    if (std::numeric_limits<unsigned>::max() == ourDistance)
        return false;

    // Get enemy military buildings in range (largest known military radius).
    for (const nobBaseMilitary* bld : aii_.gwb.LookForMilitaryBuildings(pt, HQ_RADIUS)) {
        if (bld->GetPlayer() == aii_.GetPlayerId())
            continue;
        unsigned distance = CalcDistance(bld->GetPos(), pt);
        if (distance > bld->GetMilitaryRadius())
            continue;
        if (distance < ourDistance)
            return false;
    }

    // @todo: Filter out GamePlayer::GetRestrictedArea.

    return true;
}

BuildingQuality World::AdjustBQ(const MapPoint pt, BuildingQuality nodeBQ, bool includeAnticipated) const
{
    if (nodeBQ == BQ_NOTHING || !IsPlayerTerritory(pt, includeAnticipated))
        return BQ_NOTHING;

    // If we could build a building, but the buildings flag point is at the border, we can only build a flag
    if (nodeBQ != BQ_FLAG &&
            !IsPlayerTerritory(GetNeighbour(pt, Direction::SOUTHEAST), includeAnticipated))
    {
        // Check for close flags, that prohibit to build a flag but not a building at this spot
        for (unsigned dir = Direction::WEST; dir <= Direction::NORTHEAST; ++dir) {
            if (GetBM(GetNeighbour(pt, Direction(dir))) == BlockingManner::Flag)
                return BQ_NOTHING;
        }
        return BQ_FLAG;
    } else
        return nodeBQ;
}

void World::OnBuildingNote(const BuildingNote& note)
{
    // We lost land due to the given enemy military building.
    if (note.type == BuildingNote::LostLand)
        return; // noop

    Building* bld = GetBuilding(note.pos);
    RTTR_Assert(note.type == BuildingNote::Captured || (bld && bld->GetType() == note.bld));

    switch (note.type) {
    case BuildingNote::BuildingSiteAdded:
    {
        SetState(bld, Building::UnderConstruction);
    } break;

    case BuildingNote::SetBuildingSiteFailed:
    {
        if (GetFlagState(bld->GetFlag()) == FlagRequested) {
            RemoveFlag(bld->GetFlag());
        }
        Remove(bld);
    } break;

    case BuildingNote::DestructionFailed:
    {
        if (aii_.gwb.GetNO(note.pos)->GetType() == NOP_BUILDINGSITE)
            SetState(bld, Building::UnderConstruction);
        else
            SetState(bld, Building::Finished);
    } break;

    // The construction of a building finished.
    case BuildingNote::Constructed:
    {
        SetState(bld, Building::Finished);
    } break;

    // A building was destroyed.
    case BuildingNote::Destroyed:
    {
        Remove(bld);
    } break;

    // Military building was captured.
    case BuildingNote::Captured:
    {
        // The first soldier arrived at a military building we built.
        if (bld) {
            RTTR_Assert(bld->GetState() == Building::Finished);
        }

        // We captured an enemy military building.
        else {
            bld = Create(note.bld, Building::Finished);
            SetPoint(bld, note.pos);
            FlagState state = GetFlagState(bld->GetFlag());
            (void)state;
        }

        SetCaptured(bld);
    } break;

    // A military building was captured by an enemy or destroyed.
    // There was a preceeding event for the destruction.
    case BuildingNote::Lost:
    {
        // noop
    } break;

    // Building can't find any more resources.
    case BuildingNote::NoRessources:
    {
        // @todo: It's not the job of this class to request deconstruction.
        //        Deconstruct(bld);
    } break;

    // Lua request to build this building.
    case BuildingNote::LuaOrder:
    {
        // handled in Beowulf
    } break;

    // Land lost due to enemy building.
    case BuildingNote::LostLand:
    {
        // occurs after OnRoadDestroyed()/OnFlagDestroyed()
        // noop
    } break;
    }
}

void World::OnRoadNote(const RoadNote& note)
{
    switch (note.type)
    {
    case RoadNote::Constructed:
    {
        SetRoadState(note.pos, note.route, RoadFinished);
    } break;
    case RoadNote::Destroyed:
    {
        RemoveRoad(note.pos, note.route);
    } break;
    case RoadNote::ConstructionFailed:
    {
        // Check if there was a construction site at the start.
        Building* bld = GetBuilding(GetNeighbour(note.pos, Direction::NORTHWEST));
        if (bld && bld->GetState() == Building::UnderConstruction) {
            // Is it connected in a different way?
            bool connected = false;
            for (Direction dir : Direction()) {
                if (HasRoad(note.pos, dir)) {
                    connected = true;
                    break;
                }
            }
            RTTR_Assert(connected); // This is a bug.
            Deconstruct(bld);
        }
        RemoveRoad(note.pos, note.route);
    } break;
    }
}

void World::OnFlagNote(const FlagNote& note)
{
    switch (note.type)
    {
    case FlagNote::Constructed:
    case FlagNote::DestructionFailed:
    {
        SetFlagState(note.pt, FlagFinished);
    } break;

    case FlagNote::ConstructionFailed:
    case FlagNote::Destroyed:
    {
        RemoveFlag(note.pt);
    } break;

    case FlagNote::Captured:
    {
        SetFlagState(note.pt, FlagFinished);
    } break;
    }
}

bool World::IsCloserThanEnemy(const MapPoint& pt, unsigned distance, std::vector<const nobBaseMilitary*> enemies) const
{
    for (const nobBaseMilitary* bld : enemies) {
        unsigned enemyDistance = CalcDistance(bld->GetPos(), pt);
        if (enemyDistance > bld->GetMilitaryRadius())
            continue;
        if (enemyDistance < distance)
            return false;
    }

    return true;
}

const noBaseBuilding* World::GetEnemyBuilding(const MapPoint& pt) const
{
    const MapNode& node = aii_.gwb.GetNode(pt);
    if (node.owner == aii_.GetPlayerId() + 1 || node.owner == 0)
        return nullptr;
    noBase* no = node.obj;
    if (nullptr == no)
        return nullptr;
    const NodalObjectType noType = no->GetType();
    if (noType == NOP_BUILDING || noType == NOP_BUILDINGSITE)
        return static_cast<const noBaseBuilding*>(no);
    if (noType == NOP_FLAG)
        return GetEnemyBuilding(GetNeighbour(pt, Direction::NORTHWEST));
    return nullptr;
}

std::pair<MapPoint, unsigned>
World::GroupMemberDistance(
        const MapPoint& pt,
        unsigned group,
        BuildingType type) const
{
    std::pair<MapPoint, unsigned> ret{MapPoint::Invalid(), std::numeric_limits<unsigned>::max()};

    if (group == InvalidProductionGroup)
        return ret;

    for (const Building* bld : buildings_) {
        if (bld->GetType() != type || bld->GetGroup() != group)
            continue;
        unsigned dist = CalcDistance(bld->GetFlag(), pt);
        if (dist < ret.second) {
            ret.second = dist;
            ret.first = bld->GetFlag();
        }
    }

    return ret;
}

unsigned World::GetMaxGroupMemberDistance(
        const MapPoint& pt,
        unsigned group,
        BuildingType type) const
{
    if (group == InvalidProductionGroup)
        return std::numeric_limits<unsigned>::max();

    unsigned ret = std::numeric_limits<unsigned>::max();

    for (const Building* bld : buildings_) {
        if (bld->GetType() != type || bld->GetGroup() != group || !bld->GetFlag().isValid())
            continue;
        unsigned dist = CalcDistance(bld->GetFlag(), pt);
        if (dist > ret || dist == std::numeric_limits<unsigned>::max())
            ret = dist;
    }

    return ret;
}

BlockingManner World::BQCalculator2::GetBM(const MapPoint& pt) const
{
    BlockingManner bm = world2.GetBM(pt, tmps_);
    if (bm != BlockingManner::None)
        return bm;
    return world.GetNO(pt)->GetBM();
}

} // namespace beowulf
