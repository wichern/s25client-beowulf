#!/usr/bin/env python3

import s25py

class AwesomeAI(s25py.Player):
    def run_gf(self, gf, gfisnwf):
        if gf == 100:
            print('Hello, World! My name is AwesomeAI!')

    def on_chat_message(self, playerId, msg):
        print(f'Message from {playerId}: {msg}')

game = s25py.Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd")
game.add_player(s25py.Player("dummy"))
game.add_player(s25py.PlayerAIJH("default"))
game.add_player(AwesomeAI("my"))
game.run(max_gf=10000)
