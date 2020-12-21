# Sensor

Author: Hairuo Sun, Chen-Yu Chang

Date: 2020-10-27

# Code Explanation
In this code folder, there is a main c code "udp_client.c" that connect the sensors, thermistor, accelerometer, led, and WebCam, with ESP32. Then, sensor data are sent as a comma separated string from esp32 to RPI through UDP Protocol over wifi.

# Attribution
* [UDP Client example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client)
* [UDP Server-Client implementation in C](https://www.geeksforgeeks.org/udp-server-client-implementation-c/)
* [Return a String from a C Function](https://flaviocopes.com/c-return-string/)
* [sprintf in C](https://www.geeksforgeeks.org/sprintf-in-c/)
* [C library function - strncmp()](https://www.tutorialspoint.com/c_standard_library/c_function_strncmp.htm)
* [I2C Brief](http://whizzer.bu.edu/briefs/design-patterns/dp-i2c)
* [ADXL343 Base Code](https://github.com/BU-EC444/code-examples/tree/master/i2c-accel/main)
* [ESP32 I2C example - ESP32 as master, an I2C device as slave](https://gist.github.com/mws-rmain/2ba434cd2a3f32d6d343c1c60fbd65c8)
* [ADXL343 Datasheet](https://cdn-learn.adafruit.com/assets/assets/000/070/556/original/adxl343.pdf?1549287964)
* [Adafruit Learn Guide on the ADXL343](https://learn.adafruit.com/adxl343-breakout-learning-guide/overview)
* [Tilt Sensing](https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing)
* [ESP32 WiFi Station Example Code](https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station)
* [Configuring ddclient to update your dynamic DNS at noip.com](https://www.andreagrandi.it/2014/09/02/configuring-ddclient-to-update-your-dynamic-dns-at-noip-com/)
* [Xfinity Port forwarding Setup Guide](https://www.xfinity.com/support/articles/port-forwarding-xfinity-wireless-gateway)

-----
