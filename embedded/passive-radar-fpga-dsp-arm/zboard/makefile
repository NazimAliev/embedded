# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


# use these on Linux
CP      = cp
ECHO    = echo
MKDIR   = mkdir
RM      = rm -f
RMDIR   = rm -rf

EXBASE = ..
include $(EXBASE)/products.mak

PROFILE = release
#PROFILE = debug

NE10_LIB=/home/legolas/bin/bbxm/board-support/ne10/build/modules/libNE10.a -lm -lpthread
UDP_LIB=../udp/libudp.a

#
#  ======== define all the repositories needed by this app ========
#

# root dir of our source system
ROOT_DIR ?= $(DEPOT)/../
SUFFIX          = v9A

#
#  ======== common defines used by TI build system========
#

ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(PROFILE))
ifeq (,$(wildcard bin/$(PROFILE)))
    $(shell $(MKDIR) -p bin/$(PROFILE))
endif
endif
endif

#
#  ======== toolchain macros ========
#  Note: You do not need to set CGTOOLS_V7A and CC_V7A if they have been
#  set in xdcpaths.mak.
#
CC = $(CGTOOLS_V7A)/$(CC_V7A)

CCPROFILE.debug = -g -D_DEBUG_=1 
#CCPROFILE.release = -O2
# -march=armv7-a -mtune=cortex-a8
CCPROFILE.release = -O3 -ffast-math -Ofast -ftree-vectorize -funsafe-math-optimizations

CFLAGS = -fPIC -Wall -Werror -mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 -mthumb -fno-strict-aliasing $(CCPROFILE.$(PROFILE)) \
    -I. $(addprefix -I,$(REPO_PATH))

LNKPROFILE.debug = -g
LNKPROFILE.release =
LDFLAGS = $(LNKPROFILE.$(PROFILE))
LDLIBS = -lpthread -lrt -ldl

APP.srcs = $(wildcard ./*.c)
APP.objs = $(addprefix bin/$(PROFILE)/, $(patsubst %.c,%.o$(SUFFIX),$(APP.srcs)))
APP.headers = $(patsubst %.c,%.h,$(APP.srcs))

PROGRAM = zboard

.PHONY: all install clean

#
#  ======== make commands ========
#
all:
	@$(ECHO) "# Making all $@ ..."
	@$(MAKE) bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX)


#
#  ======== build rules ========
#
bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX): $(APP.objs) 
	@$(ECHO) "# Linking $@ ..."
	@$(CC) -o $@ $^ $(NE10_LIB) $(UDP_LIB)

.SECONDARY:

bin/$(PROFILE)/%.o$(SUFFIX): %.c
	@$(ECHO) "# Compile $@ ..."
	@$(CC) $(CFLAGS) -o $@ -c $<

CROSS_COMPILE = $(subst -gcc,-,$(CGTOOLS_V7A)/$(CC_V7A))

install:
	#scp bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX) root@zb1:af_app/ 
	cp bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX) ../nfs/
	

#
#  ======== clean ========
#
clean::
	$(RMDIR) bin

