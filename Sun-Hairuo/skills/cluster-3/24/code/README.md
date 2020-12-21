# Code Readme
Author: Hairuo Sun

Date: 2020-10-19

# Code Explanation
The code includes 2 sections:
1. mcpwm functions set duty cycles from 0 up to 90 and back down to 0 once global flag equals "true". I set GPIO = 18 to output pwm signals to the LED. Then, I write the function "mcpwm_example_led_control" to update duty cycle values for the pwm signals.

2. timer functions that trigger an alarm every 250 ms. I modified the function "timer_evt_task" to set the global "flag" = true whenever the alarm is triggered.

# Attribution
* [ESP-IDF "mcpwm_servo_control" example code](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control)
* [ESP32 PWM for Motor Control](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html)
* [ESP32 LED Control](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html)
* [Stopwach Code from Skill 11](https://github.com/BU-EC444/Sun-Hairuo/tree/master/skills/cluster-1/11)
