# Code Readme
Author: Hairuo Sun

Date: 2020-10-19

# Code Explanation
In the code, I have written or modified 4 functions:
Note: Master - esp32 ; slave - accelerometer
1. writeRegister: master write 1 byte to slave.
2. readRegister: master read 1 byte from slave.
3. read16: master read 2 bytes from slave.
4. calcRP: translate accelerometer x,y,z values to roll and pitch values.


# Attribution
* [I2C Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-i2c)
* [ADXL343 Base Code](https://github.com/BU-EC444/code-examples/tree/master/i2c-accel/main)
* [ESP32 I2C example - ESP32 as master, an I2C device as slave](https://gist.github.com/mws-rmain/2ba434cd2a3f32d6d343c1c60fbd65c8)
* [ADXL343 Datasheet](https://cdn-learn.adafruit.com/assets/assets/000/070/556/original/adxl343.pdf?1549287964)
* [Tilt Sensing](https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing)
