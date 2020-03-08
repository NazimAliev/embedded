#!/bin/sh

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

# start syslink and cmem
insmod /home/root/syslink/lib/modules/3.0.50-cm-t3730/kernel/drivers/dsp/syslink.ko TRACE=1 TRACEFAILURE=1
insmod /home/root/cmem/cmemk.ko phys_start=0x8E400000 phys_end=0x8F000000 pools=20x4096,10x131072,2x1048576

CE_DEBUG=0 ./af_app.xv5T

echo clearing system virtual memory caches
echo "3" > /proc/sys/vm/drop_caches

