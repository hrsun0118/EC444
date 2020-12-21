# Code Readme
Author: Hairuo Sun

Date: 2020-11-04

# Code Explanation
I modified the original ir-car-beacon.c example code to in the following 3 sections that are changed:
1. button_task: I set a global "button_flag" and this flag is set to true whenever button is pressed.
2. send_task: I used "button_flag == true" as condition to send the signals through uart from 1 fob to the other
3. recv_task: I added a few lines of code to trigger the corresponding color led on 2nd fob once new data is received from the 1st fob.

Note: pin numbers are not modified because the same pin numbers from the example code is used when wiring up the circuits.

# Attribution
* [traffic-light-ir-example code](https://github.com/BU-EC444/code-examples/blob/master/traffic-light-ir-example/main/ir-car-beacon.c)
* [IR Communications Design Pattern](http://whizzer.bu.edu/briefs/design-patterns/dp-irtxrx)


-----
