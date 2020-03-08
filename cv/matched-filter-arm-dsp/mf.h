/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF
#define LW_AF

// grayscale 8bit image 0-255 N*N
#define N (512)
#define BUFSIZE_1D (N*2*sizeof(int16_t))
#define BUFSIZE_2D (N*N*2*sizeof(int16_t))

#endif
/*
mod.sh:
MODPATH=/lib/modules/3.0.50/kernel/drivers/dsp/

# start syslink and cmem
insmod $MODPATH/syslink.ko TRACE=1 TRACEFAILURE=1

# bbxm mem=256M version
insmod $MODPATH/cmemk.ko phys_start=0x90000000 phys_end=0x90C00000 pools=20x4096,10x131072,4x2621440

omap3530_memmap.txt:
# ALG Heap
DSP, 0x90C00000, 0x90C00000, 0x90C00000, 0x3000000, 0x5, 0
# DDR2
DSP, 0x93C00000, 0x93C00000, 0x93C00000, 0x3000000, 0x5, 0
# SR 1
DSP, 0x96E00000, 0x96E00000, 0x96E00000, 0x0200000, 0x5, 0
# SR 0
DSP, 0x96C00000, 0x96C00000, 0x96C00000, 0x0200000, 0x5, 0

=========================================================
for 512M board (BB):

Linux starts with mem=256M, so it reserved memory to:
0x80000000 (phys addr base) + 0x1000000 (256M) = 0x90000000

A least 256M available for DSP and cmem:
0x90000000 - 0xA0000000 

cmem use from 0x90000000 to 0x90C00000 for ARM-DSP exchange
4 pools because 4 big NxN arrays ARM-DSP

DSP uses mem from 0x90C00000 for internal buffers

*/

