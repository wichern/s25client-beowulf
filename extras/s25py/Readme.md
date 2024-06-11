# Install py-battle

```sh
sudo apt install python3-dev

python3 -m venv .venv
. .venv/bin/activate
pip install pybind11
```

# Run

```sh
cd build
export RTTR_PREFIX_DIR=$(pwd)
../extras/s25py/battle.py
```

