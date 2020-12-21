# Code Readme
Author: Hairuo Sun

Date: 2020-11-09

# Code Explanation
In the whack_a_mole_fsm.c file, I implemented a finite state machine with 3 states (game over, idle and mole_appear states). I also implemented 5 events: 1)start game, 2)hit mole, 3)game time_out - 20 seconds, 4)idle time_out - 2 seconds, 5)mole_appear time_out - 5 seconds). Timer and alarm are used here to decrease the time_out counts for the game.

# Attribution
* [Design Pattern for FSMs in C](http://whizzer.bu.edu/briefs/design-patterns/dp-state-machine)
* [whack-a-mole game](https://www.crazygames.com/game/whack-a-mole)


-----
