# udp_client.c

The main C code, udp_client.c, is the most important part in this quest. It transmits data to node.js through UDP. More importantly, it transmit data between ESP32s. They transmit to poll leader and when the leader fails, it will elect a new leader depending on the minimum ID. When the old leader comes back, it will again replace the new one and become a new leader. This code will run all the time going through the FSM diagram shown in the report. This code will also receive the re-election signal to start the election again.

Name: Chen-Yu Chang, Hairuo Sun
