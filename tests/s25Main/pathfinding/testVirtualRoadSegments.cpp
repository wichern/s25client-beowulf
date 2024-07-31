#include "AsciiMap.h"
#include "PointOutput.h"
#include "worldFixtures/WorldWithGCExecution.h"
#include "world/MapBase.h"
#include "gameTypes/GameTypesOutput.h"
#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_SUITE(Pathfinding)

BOOST_FIXTURE_TEST_CASE(VirtualRoadSegments, WorldWithGCExecution1P)
{
    SetupInitialRoadNetwork(world);

    AsciiMap debug(world);
    debug.drawPlayer(0);
    debug.write();
}

BOOST_AUTO_TEST_SUITE_END()
