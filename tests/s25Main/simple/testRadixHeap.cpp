// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pathfinding/RadixHeap.h"
#include "nodeObjs/noRoadNode.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestRadixHeap)

struct Node : public noRoadNode
{
    Node(unsigned e) : noRoadNode(NodalObjectType::Flag, MapPoint(0, 0), 0)
    {
        estimate = e;
    }

    void AddWare(std::unique_ptr<Ware>) override {}
    GO_Type GetGOT() const override { return GO_Type::Nothing; }
    void Draw(DrawPoint) override {}
};

BOOST_AUTO_TEST_CASE(SingleElement0)
{
    RadixHeap heap;

    BOOST_TEST_REQUIRE(heap.empty());

    Node n1(0);
    heap.insert(&n1);

    BOOST_TEST_REQUIRE(!heap.empty());

    const noRoadNode* min = heap.delete_min();

    BOOST_TEST_REQUIRE(min == &n1);
    BOOST_TEST_REQUIRE(heap.empty());
}

BOOST_AUTO_TEST_CASE(SingleElement42)
{
    RadixHeap heap;

    BOOST_TEST_REQUIRE(heap.empty());

    Node n1(42);
    heap.insert(&n1);

    BOOST_TEST_REQUIRE(!heap.empty());

    const noRoadNode* min = heap.delete_min();

    BOOST_TEST_REQUIRE(min == &n1);
    BOOST_TEST_REQUIRE(heap.empty());
}

// This is an error in Pathfinding: It's not allowed to add a key with bigger estimate than the previous!
BOOST_AUTO_TEST_CASE(SingleElementError_29)
{
    RadixHeap heap;

    BOOST_TEST_REQUIRE(heap.empty());

    Node n1(2);
    heap.insert(&n1);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 2);
    heap.insert(&n1);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 2);

    Node n2(5);
    heap.insert(&n2);
    Node n3(4);
    heap.insert(&n3);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 4);

    n1.estimate = 4;
    heap.insert(&n1);
}

BOOST_AUTO_TEST_CASE(SingleElementError_103)
{
    RadixHeap heap;

    BOOST_TEST_REQUIRE(heap.empty());

    Node n1(3);
    heap.insert(&n1);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 3);

    n1.estimate = 5;
    heap.insert(&n1);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 5);

    n1.estimate = 7;
    heap.insert(&n1);
    Node n2(6);
    heap.insert(&n2);
    BOOST_TEST_REQUIRE(heap.delete_min()->estimate == 6);
}

// BOOST_AUTO_TEST_CASE(MultipleElements)
// {
//     RadixHeap<const noRoadNode*> heap;

//     BOOST_TEST_REQUIRE(heap.empty());

//     Node nodes[4] = { 0, 16, 7, 6 };
//     for (Node& n : nodes)
//         heap.insert(&n);

//     BOOST_TEST_REQUIRE(!heap.empty());

//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[0]);
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[3]);
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[2]);
//     BOOST_TEST_REQUIRE(!heap.empty());
// }

// BOOST_AUTO_TEST_CASE(DecreaseKey)
// {
//     RadixHeap<const noRoadNode*> heap;

//     BOOST_TEST_REQUIRE(heap.empty());

//     Node nodes[6] = { 0, 5, 198561, 1251, 42, 1251 };
//     for (Node& n : nodes)
//         heap.insert(&n);

//     BOOST_TEST_REQUIRE(!heap.empty());

//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[0]);
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[1]);

//     nodes[3].estimate = 43;
//     heap.decrease_key(&nodes[3]);
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[4]);
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[3]);


//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[5]);
//     BOOST_TEST_REQUIRE(!heap.empty());
//     BOOST_TEST_REQUIRE(heap.delete_min() == &nodes[2]);
//     BOOST_TEST_REQUIRE(heap.empty());
// }

BOOST_AUTO_TEST_SUITE_END()

// b testRadixHeap.cpp:47
// b testRadixHeap.cpp:69
// b testRadixHeap.cpp:86
// b testRadixHeap.cpp:92