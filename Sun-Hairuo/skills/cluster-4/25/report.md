#  IR TX/RX

Author: Hairuo Sun

Date: 2020-11-04
-----

## Summary
In this skill, I first wired 2 identical units of both IR received and IR emitter. Then, I modified the "traffic-light-ir-example" code to achieve 2 goals:
1. Allow the led to output signal high to red, green and yellow LED consecutively and recursively for 10 seconds, 10 seconds and 2 seconds, respectively.
2. Send the color and id information through uart from 1 fob to the other so that the 2nd fob can trigger the same color led that matched with color led of the 1st fob.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/25/images/circuit.jpg">
<p>Screenshot 1: Circuit</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/25/images/console.png">
<p>Screenshot 2: Console Display</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=5aJS_jYC8Yw"><img src="https://i.ytimg.com/vi/5aJS_jYC8Yw/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: Continuous LED Display & Button Press to change Colors</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ESP32
* LED
* Timer & Alarm
* RTOS
* uart
* IR receiver (TPSO diode)
* IR transmitter (IR diode)
* ESP RMT API
* H Bridge
* Pushbutton

## Supporting Artifacts
* [GitHub repo for my skill 25 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/25/)
* [IR TX/RX Manual](http://whizzer.bu.edu/skills/ir-tx-rx)
* [TSOP38238 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Infrared/tsop382.pdf)
* [traffic-light-ir-example code](https://github.com/BU-EC444/code-examples/blob/master/traffic-light-ir-example/main/ir-car-beacon.c)
* [IR Communications Design Pattern](http://whizzer.bu.edu/briefs/design-patterns/dp-irtxrx)


-----
