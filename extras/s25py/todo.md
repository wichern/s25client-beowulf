# Todo

## API

```py
import s25py

class AwesomeAI(s25py.Player):
    def next_gameframe(self, gf, gfisnwf):
        if gf == 100:
            hqs = self.get_headquaters()
            build_locations = self.build_locations.get_nearest(hqs[0].flag_pos, BuildingQuality.House, 10)

        pass
```

## AI requirements

* MVP
** Attack building
** Attack via sea
* Optional
** Chat with player (logic implemented by user)
** Alliances (logic implemented by user)
** Different AI difficulties (logic implemented by user)

## Tasks

[ ] Define callbacks for game events
[ ] Allow setting construction sites and connecting flags
[ ] Allow selecting the AI in game


How to know where to place a woodcutter? 
This is an important part of the AI strategy. It should be implemented in the AI python code.
What it needs is to know where trees are
