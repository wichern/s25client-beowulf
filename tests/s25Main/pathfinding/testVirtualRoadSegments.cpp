#include "AsciiMap.h"
#include "PointOutput.h"
#include "worldFixtures/WorldWithGCExecution.h"
#include "world/MapBase.h"
#include "gameTypes/GameTypesOutput.h"
#include <boost/test/unit_test.hpp>

#if 0
static void SetupInitialRoadNetwork(GameWorld& world)
{
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::West, Direction::West});
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::NorthEast, Direction::NorthEast});
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::SouthWest, Direction::SouthWest});
    world.BuildRoad(0, false, MapPoint(5, 6),
                    {Direction::NorthWest, Direction::NorthWest, Direction::NorthWest, Direction::NorthWest});
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::SouthWest, Direction::West});
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::East, Direction::NorthEast});
    world.SetBuildingSite(BuildingType::Farm, MapPoint(1, 2), 0);
}
#endif

BOOST_AUTO_TEST_SUITE(Pathfinding)

BOOST_FIXTURE_TEST_CASE(VirtualRoadSegments, WorldWithGCExecution1P)
{
    // Initial World has a HQ only
    AsciiMap debug(world);
    debug.drawPlayer(0);
    debug.drawVroutes(0);
    std::cout << "Initial World" << std::endl;
    debug.write();

    // HQ and flag are not connected by a vroute
    const auto* noHQ = world.GetSpecObj<noRoadNode>(MapPoint(6, 5));
    const auto* noHQflag = world.GetSpecObj<noRoadNode>(MapPoint(7, 6));

    BOOST_REQUIRE(noHQ->getVRoutes()[Direction::SouthEast] == nullptr);
    BOOST_REQUIRE(noHQflag->getVRoutes()[Direction::NorthWest] == nullptr);

    // Adding a new road to the HQ should result in a vroute
    {
        world.BuildRoad(0, false, MapPoint(7, 6), {Direction::West, Direction::West});
        
        debug.drawPlayer(0);
        debug.drawVroutes(0);
        std::cout << "Initial World" << std::endl;
        debug.write();

        const auto* flag2 = world.GetSpecObj<noRoadNode>(MapPoint(5, 6));
        auto vroute = noHQ->getVRoutes()[Direction::SouthEast];
        BOOST_REQUIRE(vroute != nullptr);
        BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::NorthWest]);
        BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::West]);
        BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    }

    // Adding a new road to the other side breaks the vroute again.
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::NorthEast, Direction::NorthEast});
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(nullptr == flag2->getVRoutes()[Direction::East]);

    // Another road
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::SouthWest, Direction::SouthWest});
    const auto* flag3 = world.GetSpecObj<noRoadNode>(MapPoint(6, 8));
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(nullptr == flag3->getVRoutes()[Direction::NorthEast]);

    // Adding this new road creates vroute again
    world.BuildRoad(0, false, MapPoint(5, 6),
                    {Direction::NorthWest, Direction::NorthWest, Direction::NorthWest, Direction::NorthWest});
    const auto* flag4 = world.GetSpecObj<noRoadNode>(MapPoint(3, 2));
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);

    // This flag will extend the existing vroute
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::SouthWest, Direction::West});
    const auto* flag5 = world.GetSpecObj<noRoadNode>(MapPoint(1, 3));
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);

    // @todo: Check f1 and f2 of vroutes as well
    debug.drawPlayer(0);
    debug.drawVroutes(0);
    debug.write();
    return;


    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute == flag5->getVRoutes()[Direction::East]);

    // This new road will split the existing vroute into two parts
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::East, Direction::NorthEast});
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == flag4->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == flag5->getVRoutes()[Direction::East]);

    // Adding this buildingsite creates a second vroute
    world.SetBuildingSite(BuildingType::Farm, MapPoint(1, 2), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);
    auto vroute2 = flag5->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag4->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag5->getVRoutes()[Direction::NorthWest]);
    const auto* farm = world.GetSpecObj<noRoadNode>(MapPoint(1, 2));
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);

    // Adding this flag creates a vroute between noHQflag and flag4
    world.SetFlag(MapPoint(4, 4), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);
    vroute2 = flag5->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag4->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag5->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);
    const auto* flag6 = world.GetSpecObj<noRoadNode>(MapPoint(4, 4));
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::SouthEast]);

    // Removing flag 8,4 does not change the vroutes
    world.DestroyFlag(MapPoint(8, 4), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag2->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag4->getVRoutes()[Direction::SouthEast]);
    vroute2 = flag5->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag4->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag5->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::SouthEast]);

    // Removing flag3 will extend the vroute to the HQ again
    world.DestroyFlag(MapPoint(6, 8), 0);

    // @todo: Test extending a route
    // @todo: TEst removing Flag at 4,1 -> should merge two vroutes
    debug.drawPlayer(0);
    debug.drawVroutes(0);
    debug.write();
}

BOOST_AUTO_TEST_SUITE_END()
