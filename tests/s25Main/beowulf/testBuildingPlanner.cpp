// Copyright (c) 2016 - 2017 Settlers Freaks (sf-team at siedler25.org)
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
#include "worldFixtures/WorldWithGCExecution.h"

#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Debug.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/BuildLocations.h"

#include "factories/AIFactory.h"
#include "buildings/noBuildingSite.h"

#include <boost/test/unit_test.hpp>

#include "helper.h"

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_AUTO_TEST_SUITE(BeowulfBuildingPlanner)

using beowulf::Beowulf;
using beowulf::Building;
using beowulf::BuildLocations;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_FIXTURE_TEST_CASE(PlanSingleBuilding, BiggerWorldWithGCExecution)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    Building* bld = beowulf->world.Create(BLD_BREWERY, Building::PlanningRequest);
    beowulf->build.Request(bld, beowulf->world.GetHQFlag());

    Proceed([&]() { return bld->GetState() == Building::UnderConstruction; }, { beowulf.get() }, em, world);

    BOOST_REQUIRE(IsConnected(bld, beowulf.get()));
}

BOOST_FIXTURE_TEST_CASE(PlanMultipleBuildings, BiggerWorldWithGCExecution)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    std::vector<Building*> requests;
    requests.push_back(beowulf->world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FORESTER, Building::PlanningRequest));

    for (Building* building : requests)
        beowulf->build.Request(building, beowulf->world.GetHQFlag());

    Proceed([&](){ return requests.front()->GetState() == Building::UnderConstruction; }, { beowulf.get() }, em, world);

    for (Building* building : requests) {
        BOOST_REQUIRE(building->GetState() == Building::UnderConstruction);
        BOOST_REQUIRE(IsConnected(building, beowulf.get()));
    }

    // Can at least one building be finished?
    Proceed([&]() { return requests.front()->GetState() == Building::Finished; }, { beowulf.get() }, em, world);
    BOOST_REQUIRE(requests.front()->GetState() == Building::Finished);
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildingsStepByStep, BiggerWorldWithGCExecution)
{
    /**
     * Rerunning one many buildings planning step by step in order to find a bug.
     * This is still a useful unittest, as it covers the bug found.
     */
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();
    BuildLocations buildLocations(beowulf->world, true);
    buildLocations.Calculate(beowulf->world.GetHQFlag());

    /*
     * Construct Sawmill at (14:12)
     * Construct route from (14:13) to (W, NW)
     * Construct Storehouse at (7:17)
     * Construct route from (8:18) to (NE, NE, NE, NE, NE, NE, E, E)
     * Construct Bakery at (8:15)
     * Construct Forester at (9:13)
     * Construct Farm at (12:15)
     * Construct route from (13:16) to (NE, NE, NE)
     * Construct Farm at (11:18)
     * Construct route from (11:19) to (NE, NE, NE)
     * Construct Farm at (6:11)
     * Construct route from (7:12) to (SE, SE, E, E)
     * Construct Mill at (11:16)
     * Construct route from (11:17) to (NE, E)
     * Construct Woodcutter at (11:14)
     * Construct route from (11:15) to (W, NW)
     * Construct Woodcutter at (12:13)
     * Construct route from (13:14) to (W, SW)
     * Construct Well at (7:14)
     * Construct route from (7:15) to (SE, E)
     *  0  .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     *
     *  1    .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     *
     *  2  .   .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  3    .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  4  .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .
     *
     *  5    .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .
     *
     *  6  .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .
     *
     *  7    .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .   .
     *
     *  8  .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .
     *
     *  9    .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .   .
     *
     *  10 .   .   .   .   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   .   .
     *
     *  11   .   .   .   _   _   _   Far _   _   _   _   _   HQ  _   _   _   _   _   _   _   _   _   .   .
     *
     *  12 .   .   .   .   _   _   _   0   _   _   _   0---0---0   Saw _   _   _   _   _   _   _   .   .
     *                                  \             /         \
     *  13   .   .   .   .   _   _   _   0   _   For 0   _   _   0---0   _   _   _   _   _   _   .   .   .
     *                                    \         /               /
     *  14 .   .   .   .   .   _   _   Wel 0---0---0   _   _   _   0   _   _   _   _   _   _   .   .   .
     *                                            /               /
     *  15   .   .   .   .   .   _   _   0   Bak 0   _   _   Far 0   _   _   _   _   _   _   .   .   .   .
     *                                    \     /               /
     *  16 .   .   .   .   .   .   _   _   0---0   _   _   _   0   _   _   _   _   _   _   .   .   .   .
     *                                        /               /
     *  17   .   .   .   .   .   .   _   Sto 0   _   _   _   0   _   _   _   _   _   _   .   .   .   .   .
     *                                      /               /
     *  18 .   .   .   .   .   .   .   _   0   _   _   Far 0   _   _   _   _   _   _   .   .   .   .   .
     *                                                    /
     *  19   .   .   .   .   .   .   .   _   _   _   _   0   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  20 .   .   .   .   .   .   .   .   _   _   _   _   _   _   _   _   _   _   .   .   .   .   .   .
     *
     *  21   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
     */

    beowulf::Building* sawmill = beowulf->world.Create(BLD_SAWMILL, beowulf::Building::PlanningRequest);
    beowulf->world.Construct(sawmill, MapPoint(14, 12));
    beowulf->world.ConstructRoad(MapPoint(14, 13),
    { Direction::WEST, Direction::NORTHWEST });
    buildLocations.Update(MapPoint(14, 13), 3);

    Proceed({ beowulf.get() }, em, world);

    BuildLocations buildLocationsCheck(beowulf->world, true);
    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());

//    beowulf::AsciiMap map(beowulf->GetAII());
//    map.drawBuildLocations(beowulf->world, false);
//    map.write();

//    map.clear();
//    map.draw(buildLocations);
//    map.write();

//    map.clear();
//    map.draw(buildLocationsCheck);
//    map.write();

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* storehouse = beowulf->world.Create(BLD_STOREHOUSE, Building::PlanningRequest);
    beowulf->world.Construct(storehouse, MapPoint(7, 17));
    beowulf->world.ConstructRoad(MapPoint(8, 18),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(MapPoint(8, 18), 8);
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        if (bq_BuildLocations1 != bq_BuildLocations2) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* bakery = beowulf->world.Create(BLD_BAKERY, Building::PlanningRequest);
    beowulf->world.Construct(bakery, MapPoint(8, 15));
    buildLocations.Update(MapPoint(8, 15));
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* forester = beowulf->world.Create(BLD_FORESTER, Building::PlanningRequest);
    beowulf->world.Construct(forester, MapPoint(9, 13));
    buildLocations.Update(MapPoint(9, 13));
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            if (bq_Beowulf != bq_BuildLocations1) {
                beowulf::AsciiMap map(beowulf->GetAII(), 1);
                map.draw(beowulf->world, true);
                map.write();
                BOOST_REQUIRE(false);
            }
        }
    }

    Building* farm1 = beowulf->world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf->world.Construct(farm1, MapPoint(12, 15));
    beowulf->world.ConstructRoad(MapPoint(13, 16),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(MapPoint(13, 16), 4);
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        if (bq_BuildLocations1 != bq_BuildLocations2) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* farm2 = beowulf->world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf->world.Construct(farm2, MapPoint(11, 18));
    beowulf->world.ConstructRoad(MapPoint(11, 19),
    { Direction::NORTHEAST,
      Direction::NORTHEAST,
      Direction::NORTHEAST });
    buildLocations.Update(MapPoint(11, 19), 4);
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }

    Building* farm3 = beowulf->world.Create(BLD_FARM, Building::PlanningRequest);
    beowulf->world.Construct(farm3, MapPoint(6, 11));
    beowulf->world.ConstructRoad(MapPoint(7, 12),
    { Direction::SOUTHEAST,
      Direction::SOUTHEAST,
      Direction::EAST,
      Direction::EAST });
    buildLocations.Update(MapPoint(7, 12), 5);
    Proceed({ beowulf.get() }, em, world);

    buildLocationsCheck.Calculate(beowulf->world.GetHQFlag());
    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations1 = buildLocations.Get(pt);
        BuildingQuality bq_BuildLocations2 = buildLocationsCheck.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);

        BOOST_REQUIRE(bq_BuildLocations1 == bq_BuildLocations2);

        // BuildingLocations ignores flags
        if (bq_Beowulf != BQ_FLAG) {
            BOOST_REQUIRE(bq_Beowulf == bq_BuildLocations1);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildings, BiggerWorldWithGCExecution)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    std::vector<beowulf::Building*> requests;
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FARM, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_MILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FORESTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_STOREHOUSE, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WELL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_BAKERY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FARM, Building::PlanningRequest));
    //requests.push_back(buildings.Create(BLD_FARM, beowulf::Building::PlanningRequest));

    for (Building* bld : requests)
        beowulf->build.Request(bld, beowulf->world.GetHQFlag());


    Proceed([&]() { return requests.front()->GetState() == Building::UnderConstruction; }, { beowulf.get() }, em, world);

    BuildLocations bl(beowulf->world, true);
    bl.Calculate(beowulf->world.GetHQFlag()); // HQ flag

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);
        BuildingQuality bq_GWB = beowulf->GetAII().gwb.GetBQ(pt, beowulf->GetPlayerId());

        if (bq_GWB != bq_Beowulf) {
            BOOST_REQUIRE(false);
        }

        // BuildingLocations ignores flags
        if (bq_GWB != BQ_FLAG) {
            if (bq_GWB != bq_BuildLocations) {
                beowulf::AsciiMap map(beowulf->GetAII(), 1);
                map.draw(beowulf->world, true);
                map.draw(bl);
                map.write();
                BOOST_REQUIRE(false);
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(PlanManyBuildingsOnRealMap, WorldLoaded1PFixture)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    std::vector<beowulf::Building*> requests;
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FARM, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_MILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FORESTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_STOREHOUSE, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WELL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_BAKERY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FARM, Building::PlanningRequest));

    for (Building* bld : requests)
        beowulf->build.Request(bld, beowulf->world.GetHQFlag());

    Proceed([&]() { return requests.front()->GetState() == Building::UnderConstruction; }, { beowulf.get() }, em, world);

    BuildLocations bl(beowulf->world, true);
    bl.Calculate(beowulf->world.GetBuildings().front()->GetFlag());

    RTTR_FOREACH_PT(MapPoint, world.GetSize()) {
        BuildingQuality bq_BuildLocations = bl.Get(pt);
        BuildingQuality bq_Beowulf = beowulf->world.GetBQ(pt, true);
        BuildingQuality bq_GWB = beowulf->GetAII().gwb.GetBQ(pt, beowulf->GetPlayerId());

        BOOST_REQUIRE(bq_GWB == bq_Beowulf);

        // BuildingLocations ignores flags and can be same as gwb or none (because there is no route to the building flag)
        if (bq_GWB != BQ_FLAG && bq_GWB != bq_BuildLocations) {
            BOOST_REQUIRE(bq_BuildLocations == BQ_NOTHING);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(InvalidPositionBug_1, WorldLoaded1PFixture)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    std::vector<beowulf::Building*> requests;
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_WOODCUTTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_FORESTER, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_SAWMILL, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_QUARRY, Building::PlanningRequest));
    requests.push_back(beowulf->world.Create(BLD_GUARDHOUSE, Building::PlanningRequest, beowulf::InvalidProductionGroup, { 12, 92 }));

    for (Building* bld : requests)
        beowulf->build.Request(bld, beowulf->world.GetHQFlag());

    Proceed([&]() {
        for (beowulf::Building* bld : requests)
            if (bld->GetState() != Building::Finished)
                return false;
        return true;
    }, { beowulf.get() }, em, world, 10000);

    BuildLocations bl(beowulf->world, true);
    bl.Calculate(beowulf->world.GetHQFlag());

    for (beowulf::Building* bld : requests) {
        BOOST_REQUIRE(bld->GetState() == Building::Finished);
    }
}

BOOST_FIXTURE_TEST_CASE(TooManyBuildings, WorldLoaded1PFixture)
{
    std::unique_ptr<Beowulf> beowulf(static_cast<Beowulf*>(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world)));
    beowulf->DisableRecurrents();
    beowulf->build.Enable();

    std::vector<beowulf::Building*> requests;
    for (int i = 0; i < 20; ++i) {
        beowulf::Building* building = beowulf->world.Create(BLD_FARM, Building::PlanningRequest);
        requests.push_back(building);
        beowulf->build.Request(building, beowulf->world.GetHQFlag());
    }

    Proceed([&]() { return requests.front()->GetState() != Building::PlanningRequest; }, { beowulf.get() }, em, world, 1000);

    // Expect the additional buildings to be readded to the request queue.
    BOOST_REQUIRE(beowulf->build.GetRequestCount() > 0);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
