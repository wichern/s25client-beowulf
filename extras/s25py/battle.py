#!/bin/env python3

from s25py import Game, Player


class AwesomeAI(Player):
    def run_gameframe(self, gameframe, is_networkframe):
        pass

game = Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd", replay="bergschlumpf.rpl")

game.add_player(Player("dummy"))

while game.next_gameframe():
    print(f'GF : {game.current_gf}')
