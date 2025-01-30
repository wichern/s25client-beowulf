# s25py

`s25py` is an extension to `s25client` that allows running AIs written with python.

## Minimal example

```py
import s25py

class AwesomeAI(s25py.Player):
    def run_gf(self, gf, gfisnwf):
        pass

    def on_chat_message(self, playerId, msg):
        pass
```

## Run in s25client

In order for `s25client` to find your AI, you have to create a subdirectory in `<RTTR_RTTR>/assets/ai` containing a `__init__.py` (e.g. `<RTTR_RTTR>/assets/ai/AwesomeAI/__init__.py`).
`s25client` will import every class in your module that subclasses `s25py.Player`.

## Debug

In order to debug your AI with your favourite python IDE, you can run a headless game.

### Create s25py.cython*.so

Install required packages
```sh
sudo apt install python3-dev
```

Create a virtual environment to not install the package globally.
```sh
python3 -m venv .venv
. .venv/bin/activate
pip install pybind11
```

Build
```sh
cd build
cmake ..
make s25py
```

Install s25py into your virtual environment
```sh
cp lib/s25py.cpython-*.so ../.venv/lib/$(python -c "import sys; version=sys.version_info; print(f'python{version.major}.{version.minor}')")/site-packages
```

### Run a headless game

Create a game script (`my_game.py`).
```py
import s25py
import your_ai

game = s25py.Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd")
game.add_player(s25py.Player())
game.add_player(s25py.PlayerAIJH())
game.add_player(your_ai.AwesomeAI())
game.run(max_gf=10000)
```

We need to manually specify the S2 install dir, because we cannot determine it the way the `s25client` does (relative to binary dir). The binary in our case is `python3`.

```sh
chmod +x my_game.py
. .venv/bin/activate
export RTTR_PREFIX_DIR=$(pwd)
./my_game.py
```

### Game settings

The `Game` class has a list of optional properties.

* `replay`: path to replay file (default: None).
* `objective`: `s25py.Objective` (default: `s25py.GameObjective::TotalDomination`).
* `max_gameframe`: Gameframe at which the game will stop although it is not finished (default: UINT_MAX).
* `random_seed`: Seed value for random number generation (default: 0).
* `networkframe_interval`: Interval at which game frames will be network game frames (default: 20).

# Debugging s25client

```sh
RTTR_PREFIX_DIR=$(pwd) gdb --args python my_game.py
```

# Map/World Layers

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