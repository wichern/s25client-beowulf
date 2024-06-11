#!/bin/env python3

import os
import sys

sys.path.append(os.path.dirname(__file__) + "/../../build/lib")
import s25py

game = s25py.Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd")
game.objective = s25py.GameObjective.TotalDomination

# TODO: make the default player a dummy player
class MyAI(s25py.Player):
    def __init__(self, name):
        super(MyAI, self).__init__(name)
        pass

    def on_gameframe(self, gfisnwf):
        if gfisnwf:
            print('on gameframe (gfisnwf)')
        else:
            print('on gameframe')

player = MyAI("Player X")
game.add_player(player)

game.start()
for i in range(0, 10):
    game.step()

game.stop()
