# Install py-battle

## Todo

[ ] Make AwesomeAI build a building
[ ] Make AwesomeAI connect a building
[ ] Create Make target to watch last replay

[ ] Make s25client able to load python AIs
[ ] Create documentation

## Prepare

We need `python3-dev` and the `pybind11` package.

```sh
sudo apt install python3-dev

# Crate a venv to not install the package globally.
python3 -m venv .venv
. .venv/bin/activate
pip install pybind11
```

## Build

```sh
cd build
cmake ..
make s25py
```

## Install

```sh
cp lib/s25py.cpython-*.so $VIRTUAL_ENV/lib/$(python -c "import sys; version=sys.version_info; print(f'python{version.major}.{version.minor}')")/site-packages
```

## Run

We need to manually specify the S2 install dir, because we cannot determine it the way the s25client does (relative to binary dir). The binary in our case is python3.

```sh
. .venv/bin/activate
export RTTR_PREFIX_DIR=$(pwd)
./my_script.py
```

# Quickstart

## Minimal example

```py
from s25py import Game, Player, PlayerAIJH

class AwesomeAI(Player):
    def run_gameframe(self, gameframe, is_networkframe):
        pass

game = Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd")

game.add_player(Player("dummy"))
game.add_player(PlayerAIJH("default"))
game.add_player(AwesomeAI("my"))

while game.next_gameframe():
    pass
```

## Game settings

The `Game` class has a list of optional properties.

* `replay`: path to replay file (default: None).
* `objective`: `s25py.Objective` (default: `s25py.GameObjective::TotalDomination`).
* `max_gameframe`: Gameframe at which the game will stop although it is not finished (default: UINT_MAX).
* `random_seed`: Seed value for random number generation (default: 0).
* `networkframe_interval`: Interval at which game frames will be network game frames (default: 20).

# Debugging

```sh
RTTR_PREFIX_DIR=$(pwd) gdb --args python myscript.py
```


# General s25client dev tips

## Map/World Layers

Inheritance hierarchy and important members:

* `MapBase`: (`size_`)
* `World`:
** `nodes`
*** `altitude`
*** `t1`, `t2` of type `terrainDesc`
*** `resources`
*** `owner`
*** `bq` (when recalculated?)
*** `fow` by player
*** `seaId`
*** `harborId`
*** `obj` object of type `noBase*` that is located at this position
** `seas` Id and number of nodes containing the sea
** `harbor_pos` List of harbor positions
* `GameWorldBase`
** `roadPathFinder`
** `freePathFinder`
** `notifications`
** `players`
** `gameSettings`
** `em` EventManager

## Pathfinding

Ideas
1. Improve API 
2. Use Lifelong Planning A* or Bidirectionl A* to reduce search space

### Road Path Finding

Used for wares transport.

performance ideas:

  make this a reference in PathConditionReachable
* reuse PathConditionReachable::IsEdgeOk in PathConditionHuman::IsEdgeOk
* check if the order of condition checks is important.
  e.g. call IsNodeOk of baseclass first
* else if(t.Is(ETerrain::Walkable))
                goodTerrainFound = true; 
  in PathConditionReachable can be turned into
  goodTerrainFound = t.Is(ETerrain::Walkable)

AI ideas

* Add condition where we leave space for farms (maybe cost is better here)
* Add cost function to find a better than the shortest path
* Add Condition that includes anticipated buildings

### Other Path Finding

* Path Conditions (Check edge and node)
  `PathConditionReachable`
  checks if node and edge are walkable (no unreachable terrain)
* `PathConditionHuman` : `PathConditionReachable`
  Checks for blocking manners
* `PathConditionShip`
  Checks that it is not too close to the shore
* `PathConditionTrade` : `PathConditionHuman`
  checks that the land belongs to player or an ally
* `PathConditionRoad`
  checks that the edge is in player territory and that a road can be placed (no blockingmanner, not on border, no other roads, )

### Connect a flag to the road network (reuse existing roads)
`AIInterface::FindFreePathForNewRoad`

### Check if a road is possible (FloodFill)


* `GameWorldWorld` bs `GameWorldViewer`: The `GameWorldViewer` has an additional indirection in order to get the `world` object.

`FindPathForRoad(world, startPt, endPt, isBoatRoad, maxLen)`
calls `GetFreePathFinder()` on the world object and calls `FindPath` with the condition `makePathConditionRoad(world, isBoatRoad)`.

`DoesReachablePathExist(world, startPt, endPoint, maxLen)`
calls `GetFreePathFinder()` on the world object and calls `FindPath` with the condition `PathConditionReachable(world)`.

`FreePathFinder`
* `FindPath`
  A*, from start to dest.
* `FindPathAlternatingConditions`
  Uses two different conditions (abwehcseln, nicht gleichzeitig)
* `CheckRoute`
  Checks if given route<Direction> is still valid

Methods take an object of `TNodeChecker` (`bool IsNodeOk(pt)` and `bool IsEdgeOk(pt, dirFromPrevPt)`)

Path Conditions
* `PathConditionReachable` .. generally walkable but does not respect BlockingManners (Building, Single, NothingAround)
* `PathConditionHuman` .. a human can walk there (Woodcutter, Stonecutter, Scout, Soldier)
* `PathConditionRoad` .. a road to that destination can be created
* `PathConditionBoat`
* `PathConditionTrade`

`RoadPathFinder` .. Finds a path ON a road