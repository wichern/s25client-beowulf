#!/bin/env python3

from s25py import Game, Player, PlayerAIJH

class AwesomeAI(Player):
    def __init__(self, name):
        #Player.__init__(self, name)
        super().__init__(name)

    def next_gameframe(self, gf, gfisnwf):
        if gf == 100:
            hqs = self.get_headquaters()
            for hq in hqs:
                print(hq.pos)
            #build_locations = self.get_build_locations(hqs[0].position, BuildingQuality.House)

        pass

game = Game("<RTTR_RTTR>/MAPS/OTHER/Bergschlumpf.swd", replay="bergschlumpf.rpl", max_gameframe=101)

game.add_player(Player("dummy"))
game.add_player(AwesomeAI("dummy"))
#game.add_player(PlayerAIJH("dummy2"))

while game.next_gameframe():
    #print(f'GF : {game.current_gf}')
    #print(game.statistic_buildings)
    pass
