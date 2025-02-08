# s25py

`s25py` is an extension to `s25client` that enables running AI scripts written in python.

## Minimal example

```py
import s25py

class AwesomeAI(s25py.Player):
    def run_gf(self, gf, gfisnwf):
        pass

    def on_chat_message(self, playerId, dest, msg):
        pass
```

## Running in s25client

To allow `s25client` to detect your AI, create a subdirectory inside `<RTTR_RTTR>/assets/ai` containing an `__init__.py` file (e.g. `<RTTR_RTTR>/assets/ai/AwesomeAI/__init__.py`).
`s25client` will automatically import any class in your module that subclasses `s25py.Player`.

## Debugging

To debug your AI using your preferred Python ID, you can run a headless game.

### Building s25py.cython*.so

Install required packages
```sh
sudo apt install python3-dev
```

Create a virtual environment to not install the package globally.
```sh
python3 -m venv .venv
. .venv/bin/activate
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
game.add_player_obj("AI #1", your_ai.AwesomeAI())
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
