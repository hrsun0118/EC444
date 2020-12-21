#  Battery Voltage Monitor

Author: Hairuo Sun

Date: 2020-10-08
-----

## Summary
In this task, I connected a voltage divider with ADC1 and output the voltage value read from ADC pin onto the alphanumeric display. RTOS is used to set up adc_task, timer_task & alphanumeric display task. The adc reading of the voltage will be obtained every 100ms through timer trigger and the calculated voltage will be output to the console. The average voltage will be calculated every 1 sec and output to both the console and the alphanumeric display by using global flags.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/12/images/adc.png">
<p>Screenshot: ADC Output on Console</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=nY1nkhXHR8o"><img src="https://i.ytimg.com/vi/nY1nkhXHR8o/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: ADC Output on Alphanumeric Display</p>
</div>

## Modules, Tools, Source Used Including Attribution
* ADC
* RTOS
* Timer
* voltage divider (with resistors)
* Alphanumeric display

## Supporting Artifacts
* [GitHub repo for my skill 12 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-2/12/)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)

-----
