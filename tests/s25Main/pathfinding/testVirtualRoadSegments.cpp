#include "AsciiMap.h"
#include "PointOutput.h"
#include "worldFixtures/WorldWithGCExecution.h"
#include "world/MapBase.h"
#include "gameTypes/GameTypesOutput.h"
#include "pathfinding/RoadPathFinder.h"
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

bool check_flags(const std::shared_ptr<VirtualRoadSegment>& vroute, const noRoadNode* f1, const noRoadNode* f2);
bool check_flags(const std::shared_ptr<VirtualRoadSegment>& vroute, const noRoadNode* f1, const noRoadNode* f2)
{
    return (vroute->f1 == f1 && vroute->f2 == f2) || (vroute->f1 == f2 && vroute->f2 == f1);
}

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
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::West, Direction::West});

    const auto* flag56 = world.GetSpecObj<noRoadNode>(MapPoint(5, 6));
    auto vroute = noHQ->getVRoutes()[Direction::SouthEast];
    BOOST_REQUIRE(vroute != nullptr);
    BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(check_flags(vroute, noHQ, flag56));

    // Adding a new road to the other side breaks the vroute again.
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::NorthEast, Direction::NorthEast});
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(nullptr == flag56->getVRoutes()[Direction::East]);

    // Another road
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::SouthWest, Direction::SouthWest});
    const auto* flag68 = world.GetSpecObj<noRoadNode>(MapPoint(6, 8));
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(nullptr == flag68->getVRoutes()[Direction::NorthEast]);

    // Adding this new road creates vroute again
    world.BuildRoad(0, false, MapPoint(5, 6),
                    {Direction::NorthWest, Direction::NorthWest, Direction::NorthWest, Direction::NorthWest});
    const auto* flag32 = world.GetSpecObj<noRoadNode>(MapPoint(3, 2));
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag32));

    // This flag will extend the existing vroute
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::SouthWest, Direction::West});
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    const auto* flag13 = world.GetSpecObj<noRoadNode>(MapPoint(1, 3));
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag13));
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute == flag13->getVRoutes()[Direction::East]);

    // This new road will split the existing vroute into two parts
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::East, Direction::NorthEast});
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == flag13->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag32));

    // Adding this buildingsite creates a second vroute
    world.SetBuildingSite(BuildingType::Farm, MapPoint(1, 2), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag32));
    auto vroute2 = flag13->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag13->getVRoutes()[Direction::NorthWest]);
    const auto* farm = world.GetSpecObj<noRoadNode>(MapPoint(1, 2));
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute2, farm, flag32));

    // Adding this flag creates a vroute between noHQflag and flag32
    world.SetFlag(MapPoint(4, 4), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag32));
    vroute2 = flag13->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag13->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);
    const auto* flag6 = world.GetSpecObj<noRoadNode>(MapPoint(4, 4));
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute2, farm, flag32));

    // Removing flag 8,4 does not change the vroutes
    world.DestroyFlag(MapPoint(8, 4), 0);
    BOOST_REQUIRE(nullptr == noHQ->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(nullptr == noHQflag->getVRoutes()[Direction::NorthEast]);
    vroute = noHQflag->getVRoutes()[Direction::West];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute, noHQflag, flag32));
    vroute2 = flag13->getVRoutes()[Direction::East];
    BOOST_REQUIRE(nullptr != vroute2);
    BOOST_REQUIRE(vroute != vroute2);
    BOOST_REQUIRE(vroute2 == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute2 == flag13->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute2 == farm->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag6->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(check_flags(vroute2, farm, flag32));

    // Removing flag68 will extend the vroute to the HQ again
    world.DestroyFlag(MapPoint(6, 8), 0);
    vroute = noHQ->getVRoutes()[Direction::SouthEast];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(check_flags(vroute, noHQ, flag32));

    // Destroying the flag at 4,1 should merge the two vroutes
    world.DestroyFlag(MapPoint(4, 1), 0);
    vroute = noHQ->getVRoutes()[Direction::SouthEast];
    BOOST_REQUIRE(nullptr != vroute);
    BOOST_REQUIRE(check_flags(vroute, noHQ, farm));
    BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == noHQflag->getVRoutes()[Direction::West]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag56->getVRoutes()[Direction::NorthWest]);
    auto* flag44 = world.GetSpecObj<noRoadNode>(MapPoint(4, 4));
    BOOST_REQUIRE(vroute == flag44->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(vroute == flag44->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthEast]);
    BOOST_REQUIRE(vroute == flag32->getVRoutes()[Direction::SouthWest]);
    BOOST_REQUIRE(vroute == flag13->getVRoutes()[Direction::East]);
    BOOST_REQUIRE(vroute == flag13->getVRoutes()[Direction::NorthWest]);
    BOOST_REQUIRE(vroute == farm->getVRoutes()[Direction::SouthEast]);

    // Destroying the farm shortens the vroute
    vroute = flag13->getVRoutes()[Direction::East];
    world.DestroyBuilding(MapPoint(1, 2), 0);
    vroute = flag13->getVRoutes()[Direction::East];
    BOOST_REQUIRE(check_flags(vroute, noHQ, flag13));

    debug.drawPlayer(0);
    debug.drawVroutes(0);
    debug.write();
}

BOOST_FIXTURE_TEST_CASE(RoadPathFinding, WorldWithGCExecution1P)
{
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::West, Direction::West});
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::NorthEast, Direction::NorthEast});
    world.BuildRoad(0, false, MapPoint(7, 6), {Direction::SouthWest, Direction::SouthWest});
    world.BuildRoad(0, false, MapPoint(5, 6),
                    {Direction::NorthWest, Direction::NorthWest, Direction::NorthWest, Direction::NorthWest});
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::SouthWest, Direction::West});
    world.BuildRoad(0, false, MapPoint(3, 2), {Direction::East, Direction::NorthEast});
    world.SetBuildingSite(BuildingType::Farm, MapPoint(1, 2), 0);
    world.SetFlag(MapPoint(4, 4), 0);

    AsciiMap debug(world);
    debug.drawPlayer(0);
    debug.drawVroutes(0);
    std::cout << "Initial World" << std::endl;
    debug.write();

    const auto* noHQ = world.GetSpecObj<noRoadNode>(MapPoint(6, 5));
    const auto* noFarm = world.GetSpecObj<noRoadNode>(MapPoint(1, 2));
    // const auto* noHQflag = world.GetSpecObj<noRoadNode>(MapPoint(7, 6));

    // BOOST_REQUIRE_EQUAL(noFarm->GetVRouteLength(Direction::SouthEast), 3);
    // BOOST_REQUIRE_EQUAL(noHQflag->GetVRouteLength(Direction::West), 6);

    RoadPathFinder rpf(world);

    unsigned length = 0;
    RoadPathDirection firstDir;
    MapPoint firstNode;
    bool ret = rpf.FindPath(*noHQ, *noFarm, true, std::numeric_limits<unsigned>::max(), nullptr, 
        &length, &firstDir, &firstNode);

    BOOST_REQUIRE(ret);
    BOOST_REQUIRE_EQUAL(length, 710);
    BOOST_REQUIRE(firstDir == RoadPathDirection::SouthEast);
    BOOST_REQUIRE_EQUAL(firstNode, MapPoint(7, 6));

    ret = rpf.FindPathFast(*noHQ, *noFarm, true, std::numeric_limits<unsigned>::max(), nullptr, 
        &length, &firstDir, &firstNode);

    BOOST_REQUIRE(ret);
    BOOST_REQUIRE_EQUAL(length, 710);
    BOOST_REQUIRE(firstDir == RoadPathDirection::SouthEast);
    BOOST_REQUIRE_EQUAL(firstNode, MapPoint(7, 6));
}

BOOST_AUTO_TEST_SUITE_END()
