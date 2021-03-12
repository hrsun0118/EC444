# Example: timer-example

This example uses the timer group driver to generate timer interrupts at a specified alarm interval.

## Functionality Overview

* 1 timer @ 1 second interval with automatic reload
* On reaching the interval value each timer will generate an alarm
* Alarms trigger subsequent interrupts, that is tracked with messages printed on the terminal `Action!`
