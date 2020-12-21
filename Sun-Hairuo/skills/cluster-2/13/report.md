#  Themistor

Author: Hairuo Sun

Date: 2020-10-08
-----

## Summary
In this tasks, I obtained the adc reading of a themistor by connecting it to an adc input. Then, I converted this adc reading to voltage reading and eventually to the temperature reading. NTC mf52-103 is used as my themistor and a 10K resistor is also connected between ground and the thermistor GND pin to reduce the current. The adc input is read every 2 seconds using timer and alarm, with RTOS as the task manager. The voltage is displayed on console. To show that the adc reading is obtained every 2 seconds, I also used the alphanumeric display.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/13/images/thermistor_console.png">
<p>Screenshot: Thermistor Output on Console</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=O54xkBBYBlY"><img src="https://i.ytimg.com/vi/O54xkBBYBlY/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: Thermistor Output on Alphanumeric Display</p>
</div>

## Modules, Tools, Source Used Including Attribution
* ADC
* RTOS
* Timer
* Voltage divider (with resistors)
* Thermistor
* Alphanumeric display

## Supporting Artifacts
* [GitHub repo for my skill 13 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/13/)
* [Thermistors/Temperature Measurement with NTC Thermistors](https://www.jameco.com/Jameco/workshop/TechTip/temperature-measurement-ntc-thermistors.html)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)

-----
