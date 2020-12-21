# Wheel and Car Speed Sensor

Author: Hairuo Sun

Date: 2020-11-28
-----

## Summary
In this skill, I connected the Optical Detector/Phototransistor - QRD1114 with esp32. I also attached a paper wheel pattern, which is essentially 12 equally spaced circular pattern with alternating white and black color, on one of the crawler wheel. The optical emitter (LED) on the sensor send out a signal and the sensor detect the changes in the reflected light (light reflected from black and white encoder surface). Since the sensor can generate a "high" signal when encounter light reflected back from white surface, and "low" signal when encountering light reflected back from the black surface, it can generate a pulse. Based on the initial counter code from espressif, I was able to count the number of positive edges of the pulse each second and to obtain the final speed of the wheels using the equation: speed = round per sec / 1 round per 6 pos edges * Circumference(wheel).

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/32/images/circuit_on_car.jpg">
<p>Screenshot 1: Circuit and Encoder on Wheel</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/32/images/console.png">
<p>Screenshot 2: ESC Calibration & Forward/Backward Console Display</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ESP32
* GPIO
* Pulse Counter
* Timer & Alarm
* RTOS
* Encoder Pattern
* Optical Detector/Phototransistor - QRD1114
* Crawler

## Supporting Artifacts
* [GitHub repo for my skill 32 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-5/32/)
* [Wheel Speed PPT Slides](http://whizzer.bu.edu/progress/ppts/buggy-cluster2)
* [Sparkfun Guide to Optical Encoder](https://learn.sparkfun.com/tutorials/qrd1114-optical-detector-hookup-guide#example-circuit)
* [ESP32 Pulse Counter](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/pcnt.html)
* [ESP32 Encoder Template](http://whizzer.bu.edu/images/encoder.gif)

-----
