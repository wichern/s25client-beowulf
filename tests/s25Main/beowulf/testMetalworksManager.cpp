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

#include "worldFixtures/WorldWithGCExecution.h"

#include "factories/AIFactory.h"
#include "ai/beowulf/Beowulf.h"
#include "ai/beowulf/Helper.h"
#include "ai/beowulf/Debug.h"

#include "nodeObjs/noTree.h"
#include "world/GameWorldBase.h"
#include "gameTypes/Inventory.h"

#include <boost/test/unit_test.hpp>

#include <set>

#include "helper.h"

using beowulf::Beowulf;

#ifndef DISABLE_ALL_BEOWULF_TESTS

BOOST_AUTO_TEST_SUITE(BeowulfMetalworksManager)

typedef WorldWithGCExecution<1, 24, 22> BiggerWorldWithGCExecution;

BOOST_FIXTURE_TEST_CASE(Simple, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf_raw->metalworks.Enable();

    // Check that we could get a forester because of the available resources.
    BOOST_REQUIRE(beowulf_raw->metalworks.JobOrToolOrQueueSpace(JOB_FORESTER, false));

    // Build Metalworks
    beowulf::Building* building = beowulf_raw->world.Create(BLD_METALWORKS, beowulf::Building::PlanningRequest);
    MapPoint metalworksPt(15, 13);
    beowulf_raw->world.Construct(building, metalworksPt);
    beowulf_raw->roads.Connect(building);
    Proceed([&]() { return building->GetState() != beowulf::Building::UnderConstruction; }, { beowulf_raw }, em, world);

    // Add iron and boards to the inventory so that the metalworks can create tools.
    Inventory inventory;
    inventory.Add(GD_IRON, 100);
    inventory.Add(GD_BOARDS, 100);
    beowulf_raw->GetAII().GetStorehouses().front()->AddGoods(inventory, true);

    // Check that a requested scythe is being built.
    {
        unsigned count = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_SCYTHE];
        beowulf_raw->metalworks.Request(GD_SCYTHE);
        Proceed([&]() { return beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_SCYTHE] > count; }, { beowulf.get() }, em, world, 100000);
        unsigned new_count = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_SCYTHE];
        BOOST_REQUIRE(new_count == count + 1);
    }

    // Check that nothing else is beeing built.
    {
        std::array<unsigned, NUM_WARE_TYPES> goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
        for (int i = 0; i < 10000; ++i)
            Proceed({ beowulf.get() }, em, world);
        std::array<unsigned, NUM_WARE_TYPES> new_goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;

        for (GoodType tool : { GD_TONGS, GD_HAMMER, GD_AXE, GD_SAW, GD_PICKAXE, GD_SHOVEL, GD_CRUCIBLE, GD_RODANDLINE, GD_SCYTHE, GD_CLEAVER, GD_ROLLINGPIN, GD_BOW }) {
            BOOST_REQUIRE(goods[tool] == new_goods[tool]);
        }
    }

    // Check that three requested tools are beeing built.
    {
        std::array<unsigned, NUM_WARE_TYPES> goods_before = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
        beowulf_raw->metalworks.Request(GD_AXE);
        beowulf_raw->metalworks.Request(GD_HAMMER);
        beowulf_raw->metalworks.Request(GD_ROLLINGPIN);
        Proceed([&]() { return beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_ROLLINGPIN] > goods_before[GD_ROLLINGPIN]; }, { beowulf_raw }, em, world, 100000);

        std::array<unsigned, NUM_WARE_TYPES> goods_after = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;

        for (unsigned i = 0; i < NUM_WARE_TYPES; ++i) {
            if (i == GD_AXE || i == GD_HAMMER || i == GD_ROLLINGPIN) {
                BOOST_REQUIRE(goods_after[i] == goods_before[i] + 1);
            } else if (i == GD_TONGS || i == GD_SAW || i == GD_PICKAXE || i == GD_SHOVEL || i == GD_CRUCIBLE || i == GD_RODANDLINE || i == GD_SCYTHE || i == GD_CLEAVER || i == GD_BOW) {
                BOOST_REQUIRE(goods_after[i] == goods_before[i]);
            }
        }
    }

    // Check that nothing else is beeing built.
    {
        std::array<unsigned, NUM_WARE_TYPES> goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
        for (int i = 0; i < 10000; ++i)
            Proceed({ beowulf.get() }, em, world);
        std::array<unsigned, NUM_WARE_TYPES> new_goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;

        for (GoodType tool : { GD_TONGS, GD_HAMMER, GD_AXE, GD_SAW, GD_PICKAXE, GD_SHOVEL, GD_CRUCIBLE, GD_RODANDLINE, GD_SCYTHE, GD_CLEAVER, GD_ROLLINGPIN, GD_BOW }) {
            BOOST_REQUIRE(goods[tool] == new_goods[tool]);
        }
    }

    // Test that we don't crash when the metalsworks is beeing destroyed.
    {
        beowulf_raw->world.Deconstruct(beowulf_raw->world.GetBuilding(metalworksPt));
        Proceed({ beowulf.get() }, em, world);

        // Check that a requested hammer is not beeing built.
        std::array<unsigned, NUM_WARE_TYPES> goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
        beowulf_raw->metalworks.Request(GD_HAMMER);
        for (int i = 0; i < 10000; ++i)
            Proceed({ beowulf.get() }, em, world);
        std::array<unsigned, NUM_WARE_TYPES> new_goods = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
        for (GoodType tool : { GD_TONGS, GD_HAMMER, GD_AXE, GD_SAW, GD_PICKAXE, GD_SHOVEL, GD_CRUCIBLE, GD_RODANDLINE, GD_SCYTHE, GD_CLEAVER, GD_ROLLINGPIN, GD_BOW }) {
            BOOST_REQUIRE(goods[tool] == new_goods[tool]);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(ContinueQueueAfterReconstruction, BiggerWorldWithGCExecution)
{
    std::unique_ptr<AIPlayer> beowulf(AIFactory::Create(AI::Info(AI::BEOWULF, AI::HARD), 0, world));
    Beowulf* beowulf_raw = static_cast<Beowulf*>(beowulf.get());
    beowulf_raw->DisableRecurrents();
    beowulf_raw->metalworks.Enable();

    // Check that we could get a forester because of the available resources.
    BOOST_REQUIRE(beowulf_raw->metalworks.JobOrToolOrQueueSpace(JOB_FORESTER, false));

    // Build Metalworks
    MapPoint metalworksPt(15, 13);
    {
        beowulf::Building* building = beowulf_raw->world.Create(BLD_METALWORKS, beowulf::Building::PlanningRequest);
        beowulf_raw->world.Construct(building, metalworksPt);
        beowulf_raw->roads.Connect(building);
        Proceed([&]() { return building->GetState() != beowulf::Building::UnderConstruction; }, { beowulf_raw }, em, world);
    }

    // Add iron and boards to the inventory so that the metalworks can create tools.
    Inventory inventory;
    inventory.Add(GD_IRON, 100);
    inventory.Add(GD_BOARDS, 100);
    beowulf_raw->GetAII().GetStorehouses().front()->AddGoods(inventory, true);

    std::array<unsigned, NUM_WARE_TYPES> goods_before = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;
    beowulf_raw->metalworks.Request(GD_AXE);
    beowulf_raw->metalworks.Request(GD_HAMMER);
    beowulf_raw->metalworks.Request(GD_ROLLINGPIN);

    Proceed([&]() { return beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_AXE] > goods_before[GD_AXE]; }, { beowulf_raw }, em, world, 100000);

    // Destroy metalworks
    beowulf_raw->world.Deconstruct(beowulf_raw->world.GetBuilding(metalworksPt));
    Proceed({ beowulf.get() }, em, world);

    // Rebuild on new position
    {
        beowulf::Building* building = beowulf_raw->world.Create(BLD_METALWORKS, beowulf::Building::PlanningRequest);
        MapPoint metalworksPt(14, 5);
        beowulf_raw->world.Construct(building, metalworksPt);
        beowulf_raw->roads.Connect(building);
        Proceed([&]() { return building->GetState() != beowulf::Building::UnderConstruction; }, { beowulf_raw }, em, world);
    }

    // Check that three requested tools are beeing built.
    Proceed([&]() { return beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods[GD_ROLLINGPIN] > goods_before[GD_ROLLINGPIN]; }, { beowulf.get() }, em, world, 1000000);
    std::array<unsigned, NUM_WARE_TYPES> goods_after = beowulf_raw->GetAII().GetStorehouses().front()->GetInventory().goods;

    for (unsigned i = 0; i < NUM_WARE_TYPES; ++i) {
        if (i == GD_AXE || i == GD_HAMMER || i == GD_ROLLINGPIN) {
            BOOST_REQUIRE(goods_after[i] == goods_before[i] + 1);
        } else if (i == GD_TONGS || i == GD_SAW || i == GD_PICKAXE || i == GD_SHOVEL || i == GD_CRUCIBLE || i == GD_RODANDLINE || i == GD_SCYTHE || i == GD_CLEAVER || i == GD_BOW) {
            BOOST_REQUIRE(goods_after[i] == goods_before[i]);
        }
    }
}


BOOST_AUTO_TEST_SUITE_END()

#endif
