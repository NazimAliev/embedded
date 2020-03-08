# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

COMPILER_PATH=/home/legolas/rpi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin

export CROSS_COMPILE=arm-bcm2708hardfp-linux-gnueabi-
export PATH:=${COMPILER_PATH}:${PATH}
export CROSS=${COMPILER_PATH}/${CROSS_COMPILE}
export CC=${CROSS}gcc
export CXX=${CROSS}g++
export CPP=${CROSS}cpp
export LD=${CROSS}ld
export AS=${CROSS}as
export AR=${CROSS}ar
export RANLIB=${CROSS}ranlib
export STRIP=${CROSS}strip
export ARCH=arm

BINDIR=/home/legolas/rpi/export/af
#CFLAGS=-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -ffast-math -funsafe-math-optimizations -funroll-loops -O3

S = hex/shader_256.hex \
    hex/shader_512.hex \
    hex/shader_1k.hex \
    hex/shader_2k.hex \
    hex/shader_4k.hex \
    hex/shader_8k.hex \
    hex/shader_16k.hex \
    hex/shader_32k.hex \
    hex/shader_64k.hex \
    hex/shader_128k.hex \
    hex/shader_256k.hex \
    hex/shader_512k.hex \
    hex/shader_1024k.hex \
    hex/shader_2048k.hex \
    hex/shader_4096k.hex

C = mailbox.c gpu_fft.c gpu_fft_base.c gpu_fft_twiddles.c gpu_fft_shaders.c

C1D = $(C) af.c

H1D = gpu_fft.h mailbox.h 

F = -lrt -ldl -lm

all:	clean af.bin

af.bin:	$(S) $(C1D) $(H1D)
	$(CC) -Wall -o ${BINDIR}/af.bin $(F) $(C1D)

clean:
	rm -f ${BINDIR}/*.bin
	rm -f ${BINDIR}/1.dat
