# Code Readme
Author: Hairuo Sun

Date: 2020-10-08

# Code Explanation
In the "code" folder, the code "ir_range_finder.c" is included. It created 2 tasks: "timer_evt_task", "ir_adc_task"(Note: alpha_task is not used here). The timer alarm is set to 2 seconds by modifying "TIMER_INTERVAL_SEC" to 2. Global boolean flag for ir_adc_task is used and is triggered every 2 seconds to get adc reading of sensor output.

# Attribution
* [IR sensor with esp32](https://protosupplies.com/product/sharp-gp2y0a02yk0f-ir-distance-measuring-sensor-module/)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
