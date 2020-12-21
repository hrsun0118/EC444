# Code Readme
Author: Hairuo Sun

Date: 2020-11-09

# Code Explanation
I combined the ir transceiver code, the fsm code & the udp client/server into one election.c code to implement a bully algorithm for leader election. Finite state machine is used here, it has 4 states: init_election, election, leader & non_leader state. 3 Time_outs are used here: election time_out, leader time_out, heartbeat timeout (for leader to send messages to non leaders). In addition, esp32s' ID are compared to find the active esp32 with the lowest ID. LEDs are used to indicate states ( GREEN: init_election or elections states, BLUE: leader state, RED: non_leader state). Fobs (ESP32) send/receive information to/from one another through UDP sockets. There are 3 fobs in total.

If a leader failed, leader timer timeout and the re-election is started to find the new active esp32 with lowest ID. However, if the old leader comes back online, re-election is started again to re-elect the old leader to the leader state.


# Attribution
* [Bully Algorithm](https://en.wikipedia.org/wiki/Bully_algorithm)
* [Bully Algorithm Coursera](https://www.coursera.org/lecture/cloud-computing-2/1-4-bully-algorithm-K8QwJ)
* [UDP client example code](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client)
* [UDP server example code](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server)

-----
