#  Security in Connected Systems

Author: Hairuo Sun

Date: 2020-11-09
-----

## Summary
System Description: ESP32 & Raspberry are attached to the car. For updating the client about car's current status, ESP32 transmit current car status (location, speed camera images, etc) to RPI server through UDP, then server (RPI) transmit information through socket.io to the web client. For sending commands to control the car remotely, web client send control information to server (RPI) through socket.io, then RPI transmit the commands through UDP back to esp32 to control the car movement.

1. Weaknesses in the overall system: 1) client: there is no authentication on the sender of incoming information. 2) server: it can be taken over by an adversary. 3) local network: ear-dropping can happen when external devices are connected to this network. 4) Internet: If internet has no security protection, malicious attack can happen. 5) Node.js: there can be an adversary eardropping on the IP address where node.js code is receiving/sending data from/to. 6) ESP32: Control data can be altered if there is an adversary ear-dropping on esp32's IP and corresponding PORT.

2. Ways that bad guy can attack my specific system: 1) alter my sensor/control data 2) ear-dropping on my network to gain information 3) take control over my car 4) interrupt data transmissions (DOS) - flood the specific IP address and listening port with information to makes it hard for RPI to process. 5) Replace my device server with adversary's own server

3. Mitigate attacks described in point 2 above: 1) authenticate sender of the information. 2) encrypt messages. 3) authenticate sender of the information before implementing the control data on my car. 4) filter the content & prepare an extra server in case of attack. 5) authenticate the sender & receiver.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/29/images/flow_diagram.jpg">
<p>Screenshot: Flow of Information - Remote Car Control over Internet</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* Security
* ESP32
* Server
* Client
* node.js
* Internet

## Supporting Artifacts
* [GitHub repo for my skill 29 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-4/29/)
* [Protecting Wi-Fi Networks Against Hacking and Eavesdropping](https://www.trendmicro.com/vinfo/hk-en/security/news/cybercrime-and-digital-threats/security-101-protecting-wi-fi-networks-against-hacking-and-eavesdropping)
* [7 Tactics To Prevent DDoS Attacks & Keep Your Website Safe](https://phoenixnap.com/blog/prevent-ddos-attacks)


-----
