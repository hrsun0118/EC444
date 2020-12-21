#  Setup Espressif Toolchain

Author: Hairuo Sun

Date: 2020-09-06
-----

## Summary
1. I installed the CP2104 USB driver.
2. I followed the steps on Espressif Getting Started Guide: I installed the prerequisites, cloned ESP-IDF from GitHub into my Macbook's home directory ~/esp, installed the tools required, and set the environment variables.
3. I copied the "hello_world" project from ~/examples/get-started directory in IDF to the ~/esp directory, connected my ESP32 board & checked the serial communication port, navigated to my ~/hello_world project directory, configured ESP32 chip as my target, and run the project configuration utility menuconfig (note: menuconfig is note used in hello_world project).
4. I built the "hello_world" project, flashed it onto my ESP32 chip, and run the serial monitor to monitor output. From the serial monitor, I got "hello_world" output, which successful project execution, validating that my toolchain works.
5. I copied the "blink" project from ~/examples/get-started directory in IDF to the ~/esp directory as well. Then, I changed the pin out to 13 in "blink.c". Afterwards, I followed similar steps in the Step 3 & Step 4 above to build "blink" project & successfully flash the ESP32 chip. I observed that the serial monitor's outputs alternate between "Turning off the LED" & "Turning on the LED", while the Built-in red LED on the ESP32 turn off 1 sec & turn on 1 sec correspondingly.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-0/05/images/menuconfig.png">
<p>Screenshot 1: Menuconfig Interface</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-0/05/images/Hello_World_screenshot.png">
<p>Screenshot 2: Hello_World Serial Monitor Output in Terminal</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-0/05/images/Blink_screenshot.png" >
<p>Screenshot 3: Blink Serial Monitor Output in Terminal</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=p5xP-WqCqis"><img src="https://i.ytimg.com/vi/p5xP-WqCqis/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: Blink ESP32 Built-in LED</p>
</div>

## Modules, Tools, Source Used Including Attribution
* Adafruit
* ESP-IDF
* ESP32 Huzzah Module

## Supporting Artifacts
* [GitHub repo for my skill 05 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-0/05/)
* [USB Drive Installation Site - Silabs](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
* [Espressif Getting Started Guide](https://esp-idf.readthedocs.io/en/latest/get-started/index.html)
* [ESP32 Pin Mapping](http://whizzer.bu.edu/utilities/pins.h)
* [ESP32 Pinout](http://whizzer.bu.edu/images/huzzah-pinout.jpg)

-----
