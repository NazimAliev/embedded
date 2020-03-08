
# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

#  Build codec_dsp
#


EXBASE = ../..
include $(EXBASE)/products.mak

RM    = rm -f
RMDIR = rm -rf
ECHO  = echo
MKDIR = mkdir -p

OUTDIR = obj/$(PROFILE)
LIBDIR = lib/$(PROFILE)

INCS = -I$(XDC_INSTALL_DIR)/packages -I$(XDAIS_INSTALL_DIR)/packages -I$(ti.targets.elf.C64P)/include -I$(AER_INSTALL_DIR)/packages -I$(CE_INSTALL_DIR)/packages -I$(DSPLIB_INSTALL_DIR)/packages

PROG = codec_dsp
SRCS = codec_dsp.c afmath.c gen_twiddle_fft16x16.c gen_twiddle_fft16x32.c
OBJS = $(addprefix obj/$(PROFILE)/,$(patsubst %.c,%.oe64P,$(SRCS)))
DSPLIB_PATH = $(DSPLIB_INSTALL_DIR)/packages/ti/dsplib/lib/
DSPLIB_NAME = dsplib.ae64P
LIBS = $(DSPLIB_PATH)/$(DSPLIB_NAME)

.PHONY: debug release

all: debug release

debug:
	$(MAKE) PROFILE=$@ lib/debug/$(PROG).ae64P

release:
	$(MAKE) PROFILE=$@ lib/release/$(PROG).ae64P

CCOPTS_debug = -g -D_DEBUG_=1 --symdebug:dwarf
CCOPTS_release = -O2 -Dxdc_runtime_Log_DISABLE_ALL

CFLAGS = -qq -pdsw225 -pden -pds=195 --abi=eabi  -Dxdc_target_name__=C64P -Dxdc_target_types__=ti/targets/elf/std.h -eo.oe64P -mv6400+

CC = $(ti.targets.elf.C64P)/bin/cl6x
AR = $(ti.targets.elf.C64P)/bin/ar6x rq

obj/$(PROFILE)/%.oe64P: %.c
	$(RM) $@
	@$(ECHO) bin/cl6x $< ...
	$(CC) $(CFLAGS) $(CCOPTS_$(PROFILE)) $(INCS) -fr=./$(OUTDIR) -fc $<

lib/$(PROFILE)/$(PROG).ae64P: $(OBJS)
	$(RM) $@
	@$(ECHO) OBJS = $(OBJS)
	@$(ECHO) archiving $(OBJS) into $@ ...
	$(AR) $@ $(OBJS)
	touch ../server_dsp/main.c

clean::
	$(RMDIR) obj lib


#
#  ======== create output directories ========
#
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(PROFILE))
ifeq (,$(wildcard $(OUTDIR)))
    $(shell $(MKDIR) -p $(OUTDIR))
endif
ifeq (,$(wildcard $(LIBDIR)))
    $(shell $(MKDIR) -p $(LIBDIR))
endif
endif
endif
