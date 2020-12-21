#  RTOS - Hardware Interrupt

Author: Hairuo Sun

Date: 2020-09-22
-----

## Summary
In this tasks, I used a pushbutton to control 2 other units to realize 2 tasks: the 4 LEDs can count from 0 up to 15 or down from 15 to 0 in a binary format, and the alphanumeric display can display "up" or "down" depending on whether the LEDs are counting up or down. The pushbutton is used to toggle between up and down once its pressed.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-1/10/images/rtos.jpeg">
<p>Screenshot 1: RTOS setup</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=77yjJUzHVNo "><img src="https://i.ytimg.com/vi/77yjJUzHVNo/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: RTOS Implementation</p>
</div>

## Modules, Tools, Source Used Including Attribution
* FreeRTOS
* Pushbutton
* Alphanumeric display
* LEDs

## Supporting Artifacts
* [GitHub repo for my skill 10 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-1/10/)
* [Design pattern for tasks in FreeRTOS](http://whizzer.bu.edu/briefs/design-patterns/dp-tasks)
* [esp example code on real_time_stats](https://github.com/espressif/esp-idf/tree/master/examples/system/freertos/real_time_stats)

-----
