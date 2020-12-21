# Code Readme
Author: Hairuo Sun

Date: 2020-10-08

# Code Explanation
In the "code" folder, the code "thermistor.c" is included. It created 3 tasks: "timer_evt_task", "thermistor_adc_task" and "alpha_task". The timer alarm is set to 2 seconds by modifying "TIMER_INTERVAL_SEC" to 2. Global boolean flags for thermistor_adc_task and alpha_task are used and both are triggered every 2 seconds: one flag is to trigger the adc reading of sensor output; the other flag is to trigger the alphanumeric display to display the voltage reading every second.

# Attribution
* [Thermistors/Temperature Measurement with NTC Thermistors](https://www.jameco.com/Jameco/workshop/TechTip/temperature-measurement-ntc-thermistors.html)
* [esp adc 1 example code](https://github.com/espressif/esp-idf/blob/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc/main/adc1_example_main.c)
* [esp adc guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html)
