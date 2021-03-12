# I2C Alphanumeric Display Example

We took the ESP32 I2C example and reworked it to control the Alphanumeric Display in your kit. Hopefully, this will help you kickstart the Alphanumeric skill and also Quest 1. Code works as is...**most of the time (see note below)**.

## NOTE (Feb. 11, 2021)
If you get the errors shown below (or very similar) when running this code, you are running a new version of the Espressif SDK. This new version added an additional flag which must be set to allow the code to run.

### Errors

```c
E (322) i2c: i2c_param_config(644): i2c clock choice is invalid, please check flag and frequency
```
```c
E (1344) i2c: i2c_set_pin(825): scl and sda gpio numbers are the same
```

### Fix

To fix this error, either add (or uncomment) the line indicated below to the UART configuration (i2c_example_master_init()) after the clock speed is set:
```c
... 
conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
conf.clk_flags = 0;                                       // <-- ADD THIS LINE
err = i2c_param_config(i2c_master_port, &conf);           // Configure
...
```
