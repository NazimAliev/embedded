#!/bin/sh

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

# script for two vertical screens
ftsip/bin/ftsip -c 100 ftsip/bin/global.db &
ftsip/bin/ftsip -c 101 ftsip/bin/global.db &
ftsip/bin/ftsip -f 600 ftsip/bin/global.db &
ftgui/bin/ftgui 101 601 ftgui/bin/global.db &
ftgui/bin/ftgui 100 600 ftgui/bin/global.db &
