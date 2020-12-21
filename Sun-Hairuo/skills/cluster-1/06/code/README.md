# Code Readme
Author: Hairuo Sun

Date: 2020-09-22

# Changes
In the "code" folder, the code "Console.c" is included. I set the GPIO pin number to 13. The main code has 2 blocks: set up the initialize the uart & led pinout, and a while loop that loop through 3 different tasks: toggle (function: toggle_Mode()), echo(direclty output), and convert integer to hex number (function: int2hex_Mode()).

# Attribution
* [ESP32 Blink Example Code](https://github.com/espressif/esp-idf/tree/master/examples/get-started/blink)
* [Console IO Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-console)
