# Code Readme
Author: Hairuo Sun

Date: 2020-09-22

# Code Explanation
In the "code" folder, the code "mcpwm_servo_control.c" is included. It created a servo control task, initialized this mcpwm servo control unit and display tasks specified by the servo code in the while loop. I set the SERVO_MAX_DEGREE to 120 degrees, and I experimented with the min pulse width is around 600us, while the max pulse width is around 2700us. I also changed the actual function that the servo performs in the while loop so that it can rotate for a full range in both clockwise and anti-clockwise directions.

# Attribution
* [esp example code for servos](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control)
* [PWM Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-pwm)
