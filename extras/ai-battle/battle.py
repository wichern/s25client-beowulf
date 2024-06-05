#!/bin/env python3

import os
import sys

sys.path.append(os.path.dirname(__file__) + "/../../build/lib")
import s25py

game = s25py.Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd")
game.objective = s25py.GameObjective.TotalDomination

print(game.save_replay)

player_1 = s25py.Player("AIJH")
#game.AddPlayer()

