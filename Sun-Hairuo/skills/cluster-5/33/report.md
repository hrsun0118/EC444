# PID Control Using the Micro

Author: Hairuo Sun

Date: 2020-11-28
-----

## Summary
In this skill, I used LIDAR to implement my PID functions and connected LEDs to indicate error range. In this skill particularly, I implemented the P (proportional function), which is essentially getting error between set point distance and our actual distance from a wall. Then, I output this signal to the console and use LED to indicate if this error is <, > or = to 0 (red: error < 0; blue: error > 0; green: error = 0).

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/33/images/circuit.jpg">
<p>Screenshot 1: Circuit</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/33/images/console.png">
<p>Screenshot 2: Console Display of the P(proportional) of PID: Error difference</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/embed/GXQsm-2CQDk"><img src="https://img.youtube.com/vi/GXQsm-2CQDk/0.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: PID Proportional Error Display with LED signaling</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ESP32
* LIDARLite_v4LED
* I2C
* J Link & Ribbon Wire
* RTOS
* PID
* LEDs

## Supporting Artifacts
* [GitHub repo for my skill 33 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/33/)
* [PID Class PPT](http://whizzer.bu.edu/progress/ppts/buggy-cluster3)
* [PID Design Pattern](http://whizzer.bu.edu/briefs/design-patterns/dp-pid)
* [Garming Lidar v4 Datasheet](http://static.garmin.com/pumac/LIDAR-Lite%20LED%20v4%20Instructions_EN-US.pdf)
* [Garming Arduino Examples](https://github.com/garmin/LIDARLite_Arduino_Library)
* [LIDAR PPT Slides](http://whizzer.bu.edu/progress/ppts/buggy-cluster2)
* [How to Strip Ribbon Cable](https://electronics.stackexchange.com/questions/82054/how-to-strip-a-ribbon-cable)

-----
