#!/bin/env python3

from s25py import Game, Player, PlayerAIJH

class AwesomeAI(Player):
    def next_gameframe(self, gf, gfisnwf):
        if gf == 100:
            hqs = self.get_headquaters()
            build_locations = self.build_locations.get_nearest(hqs[0].flag_pos, BuildingQuality.House, 10)

        pass

game = Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd", replay="bergschlumpf.rpl", max_gameframe=101)

#game.add_player(Player("dummy"))
game.add_player(AwesomeAI("mine"))
game.add_player(PlayerAIJH("aijh"))

while game.next_gameframe():
    #print(f'GF : {game.current_gf}')
    #print(game.statistic_buildings))
    pass
