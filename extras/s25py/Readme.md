# Install py-battle

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
