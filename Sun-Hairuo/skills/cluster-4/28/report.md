#  Leader Election

Author: Hairuo Sun

Date: 2020-11-09
-----

## Summary
In this skill, I connected 3 fobs (ESP32s) through UDP to send and receive information from one another. Then, I implemented a FSM with 4 states (init_election, election, leader, non_leader states) to implement bully algorithm to select a poll leader among all the fobs.

FSM diagram state transition Explanation:
1. The init_election state is the first state when starting the program. Depending on the ID and the minimumID, one of the esp32s can be chosen as the poll leader. If myID is larger than the minimum ID, this esp32 will transfer into NON-LEADER state; however, if myID is equal to the minimum ID, it shifts to the LEADER state.
2. The poll leader also sends non-leaders messages when "heartbeat timeout" and stay in leader state if no election messages are received. If the leader receives the messages from lower/higher ID esp32s asking for re-election, it will transition to the ELECTION state.
3. The non-leader esp32s receive data from the poll leader and stay in non-leader state if it receives leader message before "leader timeout". However, it shifts to ELECTION state if "leader timeout" or if it receives election messages from other lower/higher ID esp32s.
4. In ELECTION state, higher ID esp32s send lower ID esp32s election message. If higher ID esp32s receives "ok" confirmation from lowerID esp32s, higher ID esp32s set their next state as non_leader. If an esp32 doesn't receive any "ok" confirmation, it set its own next state as leader. Then, once the election timeout, all esp32s transition to their respective next state.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/28/images/circuit.jpg">
<p>Screenshot 1: Circuit</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/28/images/FSM_design.png">
<p>Screenshot 2: FSM Diagram</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/embed/7-_57v9EbiQ"><img src="https://img.youtube.com/vi/7-_57v9EbiQ/0.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: Leader Election</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ESP32
* FSM
* WiFi
* UDP (Client & Server)
* LED
* Timer & Alarm
* RTOS
* uart
* ESP RMT API
* H Bridge
* Pushbutton

## Supporting Artifacts
* [GitHub repo for my skill 28 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/28/)
* [Bully Algorithm](https://en.wikipedia.org/wiki/Bully_algorithm)
* [Bully Algorithm Coursera](https://www.coursera.org/lecture/cloud-computing-2/1-4-bully-algorithm-K8QwJ)
* [UDP client example code](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client)
* [UDP server example code](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server)

-----
