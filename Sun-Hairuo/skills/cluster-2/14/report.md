#  Ultrasonic Sensor Reading

Author: Hairuo Sun

Date: 2020-10-08
-----

## Summary
In this tasks, I obtained the adc reading of a ultrasonic sensor by connecting it to an adc input. Then, I converted this adc reading to voltage reading and eventually to the distance reading. The adc input is read every 2 seconds using timer and alarm, with RTOS as the task manager. The voltage is displayed on console. To show that the adc reading is obtained every 2 seconds, I also used the alphanumeric display.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/14/images/ultrasonic.png">
<p>Screenshot 1: Ultrasonic Value Display on Console</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/14/images/ultrasonic_circuit.jpeg">
<p>Screenshot 2:Ultrasonic Sensor Circuit with Alphanumeric</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ADC
* RTOS
* Timer
* Voltage divider (with resistors)
* Ultrasonic Sensor
* Alphanumeric display

## Supporting Artifacts
* [GitHub repo for my skill 14 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/14/)
* [Ultrasonic sensor with esp32](https://www.instructables.com/ESP32-With-Ultrasonic-Sensor/)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)

-----
