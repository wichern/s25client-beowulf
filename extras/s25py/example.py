#!/usr/bin/env python3

import s25py

class ExampleAI(s25py.Player):
    def run_gf(self, gf, gfisnwf):
        if gf == 0:
            hq = self.get_headquater()
            print(f'Has HQ at {hq.pos}')

    def on_chat_message(self, playerId, dest, msg):
        print(f'Message from {playerId} to {dest}: {msg}')

game = s25py.Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd", objective=s25py.GameObjective.NoObjective)
game.add_player_aijh("Default AI", s25py.AILevel.Easy)
game.add_player("Python AI", s25py.AILevel.Hard, ExampleAI())
game.run(max_gf=150)
