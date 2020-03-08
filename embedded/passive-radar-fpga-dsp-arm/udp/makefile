# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

GCC=/usr/bin/arm-linux-gnueabihf-gcc
AR=/usr/bin/arm-linux-gnueabihf-ar

GCCx86=gcc
ARx86=ar

ARM_CFLAGS=-march=armv5 -mthumb -mfloat-abi=hard -mfpu=neon

CFLAGS= -Wall -Werror 

all: libudp.a libudpx86.a

# ARM version of library
udp.o:  udp.c
	@echo "udp.c -> udp.o"
	@$(GCC) $(CFLAGS) -c -fPIC udp.c -o udp.o

libudp.a: udp.o
	@echo "make libudp.a"
	@$(AR) rcs libudp.a udp.o

# x86 version of library
udpx86.o:  udp.c
	@echo "udp.c -> udpx86.o"
	@$(GCCx86) $(CFLAGS) -c -fPIC udp.c -o udpx86.o

libudpx86.a: udpx86.o
	@echo "make libudpx86.a"
	@$(ARx86) rcs libudpx86.a udpx86.o

clean:
	rm udp.o
	rm udpx86.o
	rm libudp.a
	rm libudpx86.a
