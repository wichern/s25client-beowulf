# Todo

## API

awesomeai.py
```py
import s25py

class AwesomeAI(s25py.Player):
    def next_gameframe(self, gf, gfisnwf):
        if gf == 100:
            print("Hello, World! My name is AwesomeAI")
```

GameLobbyController::TogglePlayerState() shall also list python AIs.
Therefore, AI::Info struct must be extended (A new type ("pyAI") and a path).
A global singleton (libs/s25main/ai/AILoader.h) shall provide a list of available AIs.


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
