# Code Readme
Author: Hairuo Sun

Date: 2020-10-08

# Code Explanation
In the "code" folder, the code "vadc_monitor.c" is included. It created 3 tasks: "timer_evt_task", "adc_task" and "alpha_task". The timer alarm is set to 100ms by modifying "TIMER_INTERVAL_SEC" to 0.1. Global boolean flags for adc_task and alpha_task are used and both are triggered every 1 second: one flag is to trigger the calculation of average voltage from the 10 readings within a second; the other flag is to trigger the alphanumeric display to display the voltage reading every second.

# Attribution
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
