#!/bin/bash

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


ftsip/bin/ftsip -c 100 ftsip/trunk/global.db &
ftsip/bin/ftsip -r 301 200 ftsip/trunk/global.db &

sleep 1

# call radio from cwp1
C1="201;cwp2@192.168.52.2:5060;;sendrecv:radio:RSSI:123.4;call radio from cwp1"
sudo sendip -d "${C1}" -p ipv4 -p udp -ud 7772 192.168.52.1

sleep 1

# change radio parms 
C2="201;cwp2@192.168.52.2:5060;;recvonly:radio:RSSI:145.6;change parms"
sudo sendip -d "${C2}" -p ipv4 -p udp -ud 7772 192.168.52.1

sleep 1

# terminame call from cwp1 to cwp2
C3="203;cwp2@192.168.52.2:5060;;;terminate from cwp1 to radio"
sudo sendip -d "${C3}" -p ipv4 -p udp -ud 7772 192.168.52.1

killall ftsip
