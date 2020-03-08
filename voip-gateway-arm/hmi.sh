#!/bin/sh

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

clear
# script for two vertical screens

ftsip/bin/ftsip -c 100 ftsip/trunk/global.db &
ftsip/bin/ftsip -c 101 ftsip/trunk/global.db &

fthmi/bin/fthmi 100 600 fthmi/trunk/global.db &
fthmi/bin/fthmi 101 601 fthmi/trunk/global.db &
