# Code Readme
Author: Hairuo Sun

Date: 2020-09-22

# Code Explanation
In the "code" folder, the code "RTOS.c" is included. It created 3 tasks: "test_LEDs", "pushbutton" and "test_alpha_display". The Global variable is used so that a pushbutton flag can be used to control 2 functions when the pushbutton is pressed. These 2 functions are: "test_LEDs" and "test_alpha_display" so that the counting up/down of LEDs and displaying "UP"/"DOWN" on the alphanumeric display can happen simultaneously. Each task is assigned different priorities so that tasks can be managed accordingly.

# Attribution
* [Design pattern for tasks in FreeRTOS](http://whizzer.bu.edu/briefs/design-patterns/dp-tasks)
* [esp example code on real_time_stats](https://github.com/espressif/esp-idf/tree/master/examples/system/freertos/real_time_stats)
