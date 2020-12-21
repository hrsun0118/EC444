# Code Readme
Author: Hairuo Sun

Date: 2020-09-22

# Code Explanation
In the "code" folder, the code "stopwatch.c" is included. It created 3 tasks: "timer_evt_task", "pushbutton" and "test_alpha_display". Global variables for pushbutton and the count-up number are used. A pushbutton flag can be used to start/reset the counting up process. The "timer_evt_task" will be performed every second when the alarm is trigger. Once the counting up starts, the "test_alpha_display" function will display the counting process, and when the counting reaches 100, it wraps around and restart the counting from 0.

# Attribution
* [Timer Interrupts Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-timer)
* [esp example code on timer (1 second alarm)](https://github.com/BU-EC444/code-examples/tree/master/timer-example)
