#  Use PWM to Control Power Delivery to LEDs

Author: Hairuo Sun

Date: 2020-10-19
-----

## Summary
In this task, I connected an LED with resistor to ESP32 GPIO pin 18. Then, I wrote the code to let esp32 output pwm signals with different duty cycles to LED light. In fact the duty cycles increases and decreases consecutively and iteratively so that the LED's intensity level increases/decreases every 250ms. The code is based on mcpwm_servo_control code and the RTOS code written previously from skill 19.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/24/images/led_circuit.jpg">
<p>Screenshot 1: LED Circuit</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/24/images/console_output.png">
<p>Screenshot 2: Console showing LED Intensity Levels</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=JdF6yE8I32U"><img src="https://i.ytimg.com/vi/JdF6yE8I32U/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: Video  LED Intensity Going Ups & Downs</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ESP32
* LED
* MCPWM
* Timer & Alarm
* RTOS

## Supporting Artifacts
* [GitHub repo for my skill 24 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/24/)
* [ESP-IDF "mcpwm_servo_control" example code](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control)
* [ESP32 PWM for Motor Control](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html)
* [ESP32 LED Control](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html)
* [Stopwach Code from Skill 11](https://github.com/BU-EC444/Sun-Hairuo/tree/master/skills/cluster-1/11)

-----
