# Code Readme
Author: Hairuo Sun

Date: 2020-10-08

# Code Explanation
In the "code" folder, there are 4 sections: node_modules, package-lock.json, package.json and serial_node_print.js file. The node_modules contains that modules we need to use for our .js file. The package-lock.json is auto generated, and package.json is generated though "nmp init --yes" to add dependencies. The file that I edited is serial_node_print.js file. This file first read from a serial port, which is the port connected to esp32 with flashed program of reading thermistor sensor value; then, it write the data to the console.

Note: thermistor.c is included here because it is used to flash the esp32.

# Attribution
* [Node.js tutorial](https://www.w3schools.com/nodejs/default.asp)
* [IOT sensor with esp32 and node.js](https://www.twilio.com/blog/2018/07/watch-iot-sensors-esp32-javascript-nodejs-twilio-sync.html)
* [Thermistors/Temperature Measurement with NTC Thermistors](https://www.jameco.com/Jameco/workshop/TechTip/temperature-measurement-ntc-thermistors.html)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
