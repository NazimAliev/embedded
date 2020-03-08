#!/bin/sh

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

sudo ip link add veth0 type veth peer name veth1
sudo ifconfig veth0 192.168.52.1 netmask 255.255.255.0
sudo ifconfig veth1 192.168.52.2 netmask 255.255.255.0

#ip link del veth0
