# Code Readme
Author: Hairuo Sun

Date: 2020-11-28

# Code Explanation
Based on the initial MCPWM code, I added 2 functions: 1) send PWM signals to the ESC so that I can calibrate the ESC and allow the crawler to move forward/backward on demand, 2) send PWM signals to the steering to control the crawler to steer left, steer right or stop in the middle. The ESC PWM signal's duty cycle is gradually increased/decreased to reach the max/min crawler moving speed. In addition, the PWM signal sent to ESC for speed reduction action need to called twice so that the crawler can perform actual backward motion; this is due to this particular ESC's feature. 


# Attribution
* [ESC Design Pattern](http://whizzer.bu.edu/briefs/design-patterns/dp-esc-buggy)
* [ESP Example Code for Servo](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control)
* [Buggy ESC User Manual](https://www.hobbywing.com/products/enpdf/QuicRunWP1625-WP860-WP1060.pdf)
* [Buggy PPT Slides](http://whizzer.bu.edu/progress/ppts/buggy-cluster1)

-----
