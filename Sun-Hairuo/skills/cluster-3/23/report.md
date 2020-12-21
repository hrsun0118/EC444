#  Triple-axis Accelerometer

Author: Hairuo Sun

Date: 2020-10-19
-----

## Summary
In this task, I connected the accelerometer ADXL343 with ESP32 and used I2C protocol for writing and reading the accelerometer values. I wrote the read, write functions to write/read 1 byte and read 2 bytes from ADXL343 to ESP32. I also modified another translation function to translate them into tilt data. I successfully obtained the tilt data eventually.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/23/images/accel_circuit.jpg">
<p>Screenshot 1: Accelerometer ADXL343 Circuit with ESP32</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/23/images/console_output.png">
<p>Screenshot 2: Console Output Showing Accelerometer and Tilt Data</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* esp32
* ADXL343
* I2C
* Tilt Sensing

## Supporting Artifacts
* [GitHub repo for my skill 23 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/23/)
* [I2C Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-i2c)
* [ADXL343 Base Code](https://github.com/BU-EC444/code-examples/tree/master/i2c-accel/main)
* [ESP32 I2C example - ESP32 as master, an I2C device as slave](https://gist.github.com/mws-rmain/2ba434cd2a3f32d6d343c1c60fbd65c8)
* [ADXL343 Datasheet](https://cdn-learn.adafruit.com/assets/assets/000/070/556/original/adxl343.pdf?1549287964)
* [Adafruit Learn Guide on the ADXL343](https://learn.adafruit.com/adxl343-breakout-learning-guide/overview)
* [Tilt Sensing](https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing)

-----
