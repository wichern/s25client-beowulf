#!/bin/env python3

from s25py import Game, Player, PlayerAIJH

class AwesomeAI(Player):
    def run_gameframe(self, gameframe, is_networkframe):
        pass

game = Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd", replay="bergschlumpf.rpl", max_gameframe=10000)

#game.add_player(Player("dummy"))
game.add_player(PlayerAIJH("dummy2"))
game.add_player(AwesomeAI("dummy"))

while game.next_gameframe():
    #print(f'GF : {game.current_gf}')
    print(game.statistic_buildings)
