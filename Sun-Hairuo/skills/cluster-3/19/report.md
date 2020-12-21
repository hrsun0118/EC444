#  Pi as WIFI AP

Author: Hairuo Sun

Date: 2020-10-11
-----

## Summary
In this task, I configured the Raspherry Pi (RPI) as a WIFI Access Point. I deleted the "wpa_supplicant.config" file, enableed "Ethernet Gadget" by editing "config.txt" and "cmdline.txt" file in "boot" folder when SD card is inserted in the PC directly. Since SSH is already enable in the Skill 18, no modification of ssh is needed. I enabled connection sharing so that the RPI can share my PC's Internet Connection to my local router. Finally, I inserted the SD card back into the RPI, ssh into my "raspberrypi.local" connection, configured my RPI LAN with my new IP address, ssid and channel, reboot it and eventually I got my RPI as my wireless AP. This new local AP connection can be used as a regular WIFI and accessed by any other devices, such as my iPhone and iPad.

## Sketches and Photos
<div align="center">
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/19/images/network_connection.png">
<p>Screenshot 1: RNDIS/Ethernet Gadget Connection Confirmed on Macbook Setting</p>
<br/>
<br/>
<br/>
<img src="https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/19/images/wifi_connection.png">
<p>Screenshot 2: Using iPad to Access WIFI though the AP using Raspberry PI</p>
<br/>
<br/>
<br/>
<a href="https://www.youtube.com/watch?v=u56gMJZWwkE"><img src="https://i.ytimg.com/vi/u56gMJZWwkE/hqdefault.jpg" alt="IMAGE ALT TEXT"></a>
<p>Video: A Video Demonstrating that RPI as AP Works on iPad</p>
<br/>
<br/>
<br/>
</div>

## Modules, Tools, Source Used Including Attribution
* Raspberry PI
* ssh
* Raspbian
* SD card
* Ethernet Gadget

## Supporting Artifacts
* [GitHub repo for my skill 19 completion](https://github.com/BU-EC444/Sun-Hairuo/blob/master/skills/cluster-3/19/)
* [Ethernet Gadget](https://learn.adafruit.com/turning-your-raspberry-pi-zero-into-a-usb-gadget/ethernet-gadget)
* [WIFI Channel Assignment](http://whizzer.bu.edu/utilities/wifi-channels)
* [Setting Up a RPI](https://gist.github.com/superdodd/06b91ba03899e47beb43078b27dc601e)
* [HEADLESS RASPBERRY PI 3 B+ SSH WIFI SETUP](https://desertbot.io/blog/headless-raspberry-pi-3-bplus-ssh-wifi-setup)
* [SSH (Secure Shell)](https://www.raspberrypi.org/documentation/remote-access/ssh/)

-----
