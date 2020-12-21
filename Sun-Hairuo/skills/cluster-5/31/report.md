# Brining Up the LIDAR

Author: Hairuo Sun

Date: 2020-11-28
-----

## Summary
In this skill, I connected the ESP32 with the LIDARLite_v4LED sensor using J-Link socket and ribbon wires. The goal is to measure the distance from obstacle in front of the sensor. I2C protocol is used to read/write data from/to this sensor. This sensor is fairly accurate. However, the ribbon wire is connected to the male-female wire directly and has created bad contact frequently. Therefore, I used heat shrink to ensure better wire contact.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/31/images/circuit.jpg">
<p>Screenshot 1: LIDAR Circuit</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/31/images/console.png">
<p>Screenshot 2: LIDARLite_v4LED Distance Console Display</p>
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

## Supporting Artifacts
* [GitHub repo for my skill 31 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/31/)
* [Garming Lidar v4 Datasheet](http://static.garmin.com/pumac/LIDAR-Lite%20LED%20v4%20Instructions_EN-US.pdf)
* [Garming Arduino Examples](https://github.com/garmin/LIDARLite_Arduino_Library)
* [LIDAR PPT Slides](http://whizzer.bu.edu/progress/ppts/buggy-cluster2)
* [How to Strip Ribbon Cable](https://electronics.stackexchange.com/questions/82054/how-to-strip-a-ribbon-cable)

-----
