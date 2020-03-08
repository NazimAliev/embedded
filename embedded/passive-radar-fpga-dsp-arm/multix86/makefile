# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


# use these on Linux
CP      = cp
ECHO    = echo
MKDIR   = mkdir
RM      = rm -f
RMDIR   = rm -rf

#PROFILE = release
PROFILE = debug

UDP_LIB=../udp/libudpx86.a

SUFFIX          = 86 

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
CC = gcc

CCPROFILE.debug = -g -D_DEBUG_=1 
CCPROFILE.release = -O3 -ffast-math -Ofast -ftree-vectorize -funsafe-math-optimizations

CFLAGS = -fPIC -Wall -Werror -fno-strict-aliasing $(CCPROFILE.$(PROFILE)) \
    -I. $(addprefix -I,$(REPO_PATH))

LNKPROFILE.debug = -g
LNKPROFILE.release =
LDFLAGS = $(LNKPROFILE.$(PROFILE))
LDLIBS = -lpthread -lm

APP.srcs = $(wildcard ./*.c)
APP.objs = $(addprefix bin/$(PROFILE)/, $(patsubst %.c,%.o$(SUFFIX),$(APP.srcs)))
APP.headers = $(patsubst %.c,%.h,$(APP.srcs))

PROGRAM = multi

.PHONY: all clean

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
	@$(CC) -o $@ $^ $(UDP_LIB) $(LDLIBS)

.SECONDARY:

bin/$(PROFILE)/%.o$(SUFFIX): %.c
	@$(ECHO) "# Compile $@ ..."
	@$(CC) $(CFLAGS) -o $@ -c $<


#
#  ======== clean ========
#
clean::
	$(RMDIR) bin

