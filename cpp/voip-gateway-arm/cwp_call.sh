#!/bin/bash

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

ftsip/bin/ftsip -c 100 ftsip/trunk/global.db &
ftsip/bin/ftsip -c 101 ftsip/trunk/global.db &

sleep 1

# call cwp2 from cwp1
C1="200;cwp2@192.168.52.2:5060;;;call cwp2 from cwp1"
sudo sendip -d "${C1}" -p ipv4 -p udp -ud 7772 192.168.52.1

sleep 1

# cwp2 show ringing to cwp1
C2="204;cwp1@192.168.52.1:5060;;;cwp2 show ringing to cwp1"
sudo sendip -d "${C2}" -p ipv4 -p udp -ud 7772 192.168.52.2

sleep 1

# cwp2 answer to cwp1
C3="205;cwp1@192.168.52.1:5060;;;cwp2 answer to cwp1"
sudo sendip -d "${C3}" -p ipv4 -p udp -ud 7772 192.168.52.2

sleep 3

# terminame call from cwp1 to cwp2
C4="203;cwp2@192.168.52.2:5060;;;terminate from cwp1 to cwp2"
sudo sendip -d "${C4}" -p ipv4 -p udp -ud 7772 192.168.52.1

killall ftsip
