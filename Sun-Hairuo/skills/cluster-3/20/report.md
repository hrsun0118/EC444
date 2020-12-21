#  Dynamic DNS

Author: Hairuo Sun

Date: 2020-10-18
-----

## Summary
In this task, I installed ddClient on my RPI, registered a personal domain on noip.com "hrsun.ddns.net". Then, I set up ddClient to dynamically update my domain IP address to match with my home router's external IP address. I also set up the port forwarding function on my home Xfinity router site to forward traffic to my RPI, which is configured to my home router's wifi through ssid and password. Finally, I installed node.js on my RPI and run the "hello_world.js" on this RPI, and I successfully log onto my personal domain with the listening port specified in "hello_world.js". Images below can show you the successful ddns and port forwarding setup and correct domain output.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/20/images/ddns_setup.png">
<p>Screenshot 1: DDNS Address Update Message on Console and IP Address Confirmation on Noip.com</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/20/images/port_forward_setup.png">
<p>Screenshot 2: Port Forwarding Setup on Xfinity Website</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/20/images/hrsun_website.jpg">
<p>Screenshot 3: hello_world.js Output on Personal Domain (With Correct Port) "hrsun.ddns.net:1130"</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* ddclient
* Noip.com
* Xfinity
* node.js
* Raspberry PI
* ssh

## Supporting Artifacts
* [DDNS: Accessing your Raspberry Pi from Internet](https://domoticproject.com/accessing-raspberry-ddns/)
* [How to setup DDClient](https://www.youtube.com/watch?v=iuDCuUEmKF4)
* [NOIP website for Individual Domain Configuration](https://my.noip.com/)
* [Configuring ddclient to update your dynamic DNS at noip.com](https://www.andreagrandi.it/2014/09/02/configuring-ddclient-to-update-your-dynamic-dns-at-noip-com/)
* [Xfinity Port forwarding Setup Guide](https://www.xfinity.com/support/articles/port-forwarding-xfinity-wireless-gateway)
* [Setting up a wireless LAN via the command line](https://www.raspberrypi.org/documentation/configuration/wireless/wireless-cli.md)
* [Node.js HTTP tutorial](https://www.w3schools.com/nodejs/nodejs_http.asp)

-----
