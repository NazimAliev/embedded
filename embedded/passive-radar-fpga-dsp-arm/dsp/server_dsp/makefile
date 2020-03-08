# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

#
#  ======== makefile ========
#  GNUmake-based makefile
#
# dsplib notes:
# .a64P is a COFF library archive for little-endian C64x+ cores
# .a64Pe is a COFF archive for big-endian
# .ae64P is an ELF archive for little-endian
# .ae64Pe is an ELF archive for big-endian

.PHONY: all

EXBASE = ../..
include $(EXBASE)/products.mak

#profile of server
#PROFILE = release
PROFILE = debug

# Configuro settings. 

#  CONFIGPKG is the base name of the configuration script (server.cfg)
CONFIGPKG = server
CONFIG_BLD := ./_config.bld

# Name the xdc target and platform
XDCTARGET = ti.targets.elf.C64P

XDCPLATFORM = ti.platforms.evm3530:DSP

#
#  TODO:  We shouldn't have to set this, but I want to use my ce-u as my CE_INSTALL_DIR and
#  ce-u/src as the CE_REPO.
#
CE_REPO         ?= $(CE_INSTALL_DIR)/packages

#XDC_PATH := $(XDC_INSTALL_DIR)/packages;$(CE_REPO);$(BIOS_INSTALL_DIR)/packages;$(IPC_INSTALL_DIR)/packages;$(XDAIS_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/packages;$(OSAL_INSTALL_DIR)/packages;$(EXAMPLES_ROOTDIR);$(LINK_INSTALL_DIR)/packages;$(XDC_PATH)
XDC_PATH := $(XDC_INSTALL_DIR)/packages;$(CE_REPO);$(BIOS_INSTALL_DIR)/packages;$(IPC_INSTALL_DIR)/packages;$(XDAIS_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/packages;$(OSAL_INSTALL_DIR)/packages;$(LINK_INSTALL_DIR)/packages;$(XDC_PATH)

REPO_PATH = $(subst ;, ,$(XDC_PATH))

# The path to the configuro utility
CONFIGURO = XDCPATH="$(XDC_PATH)" $(XDC_INSTALL_DIR)/xs xdc.tools.configuro

COMPILER_FILE = $(CONFIGPKG)/compiler.opt
LINKER_FILE = $(CONFIGPKG)/linker.cmd

#
#  Location of codec header files and libraries to build into the server.
#  The header files are located in the directory $(CODECS_REPO), set in
#  products.mak to $(XDAIS_INSTALL_DIR)/examples.
#  Typically, codec packages would come with pre-built libraries,
#  and we would set CODEC_LIBS to -l"$(CODECS_REPO)/lib/... " instead of
# -l"../codec/lib/...".
#
CODECS_REPO ?=
CODEC_LIBS = -l"../codec_dsp/lib/$(PROFILE)/codec_dsp.ae64P"
DSPLIB_PATH = $(DSPLIB_INSTALL_DIR)/packages/ti/dsplib/lib/
#DSPLIB_NAME = dsplib.ae64P
DSPLIB_NAME = dsplib.lib
DSP_LIBS = -l"$(DSPLIB_PATH)/$(DSPLIB_NAME)"



PROGRAM = bin/$(PROFILE)/server_dsp.xe64P



CCPROFILE.release = -O2 -Dxdc_runtime_Log_DISABLE_ALL
CCPROFILE.debug = -g -D_DEBUG_=1 --symdebug:dwarf

# Use this for 'checked' support for CE extensions which support it.
#  Enabling this will result in slower library execution,
#  but it will provide extra parameter validation and trace statements.
#  It is recommended that this be enabled during early development (and
#  occasionally thereafter!) to catch common usage errors.
#

CPPFLAGS = -Dxdc_target_name__=C64P \
    -Dxdc_target_types__=ti/targets/elf/std.h $(addprefix -I,$(CODECS_REPO))

# Augment the standard $(CPPFLAGS) variable, adding the
# Configuro-generated compiler file.
CPPFLAGS +=  $(shell cat $(COMPILER_FILE))

LDFLAGS = $(LNKPROFILE.$(PROFILE)) -w -q -u _c_int00 -c \
	$(addprefix -I,$(REPO_PATH))

LDLIBS = -l $(ti.targets.elf.C64P)/lib/rts64plus_elf.lib

LNKPROFILE.debug = -q --compress_dwarf
LNKPROFILE.release = -q --compress_dwarf

#  ======== toolchain macros ========
#
#
# ti.targets.elf.C64P is the location of your C64P CGtools.
#  Check to make sure this variable is set.
#
ifneq (clean,$(MAKECMDGOALS))
ifeq (,$(wildcard $(ti.targets.elf.C64P)))
    $(error ti.targets.elf.C64P  is set to "$(ti.targets.elf.C64P)" which is invalid. Please edit the products.mak file and set this variable to the cgtools location)
endif
endif

CC = $(ti.targets.elf.C64P)/bin/cl6x
LNK = $(ti.targets.elf.C64P)/bin/lnk6x

#
#  Source files.  Note that only main.c is located in this directory. The
#  other source files are located in $(CE_REPO)/ti/sdo/ce/utils/rtcfg/bios.
#  We use the vpath variable (set below) to locate the other files.  If
#  needed, files can be copied from $(CE_REPO)/ti/sdo/ce/utils/rtcfg/bios
#  to this directory, and modified.
#
APP.srcs =                      \
	svr_rtcfg.c             \
	rtcfg_bios_fcinit.c     \
	rtcfg_bios_ceinit.c     \
	svr_config.c            \
	main.c

APP.objs = $(addprefix bin/$(PROFILE)/,$(patsubst %.c,%.oe64P,$(APP.srcs)))

#
#  This linker command file is located in $(CE_REPO)/ti/sdo/ce/utils/rtcfg/bios
#  The vpath variable is set to this directory so that make will find it. If
#  desired, this linker command file can be overridden, by copying it to this
#  directory and modifying it as needed.
#
SVR_LINKER_FILE = svr_$(PROFILE)_e64P.cmd

#
#  Set the vpath variable for make to locate .c and .cmd files not in this
#  directory.
#
vpath %.c $(CE_REPO)/ti/sdo/ce/utils/rtcfg/bios
vpath %.cmd $(CE_REPO)/ti/sdo/ce/utils/rtcfg/bios

#
#  ======== make commands ========
#
all: $(CONFIGPKG)/linker.cmd
	$(MAKE) $(PROGRAM)


# Link
$(PROGRAM):  $(APP.objs) $(LINKER_FILE) $(SVR_LINKER_FILE)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(LNK) -o $@ $(LDFLAGS) $^ $(LDLIBS) --reread_libs $(CODEC_LIBS) $(DSP_LIBS) -m $@.map

# Compile
bin/$(PROFILE)/%.oe64P : %.c $(LINKER_FILE)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
		$(CC) -c $(CPPFLAGS) $(CCPROFILE.$(PROFILE)) -eo.oe64P -fr=bin/$(PROFILE) $<


# The generated linker and compiler files depend on the generated
# config package
$(LINKER_FILE) $(COMPILER_FILE) : $(CONFIGPKG).cfg

# The generated config package depends on the 2 config scripts
$(CONFIGPKG)/linker.cmd : $(CONFIGPKG).cfg $(CONFIGBLD)
	$(CONFIGURO) -c $(ti.targets.elf.C64P) -o $(CONFIGPKG) -t $(XDCTARGET) \
	-p $(XDCPLATFORM) -b $(CONFIG_BLD) -r $(PROFILE) $(CONFIGPKG).cfg


install:
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	#scp $(PROGRAM) root@bb1:af_app/
	cp $(PROGRAM) ../../nfs/

#
#  ======== clean ========
#
clean::
	@$(ECHO) "# Removing $(CONFIGPKG) directory..."
	$(RMDIR) $(CONFIGPKG)
	@$(ECHO) "# Removing bin directory..."
	$(RMDIR) bin


#  ======== standard macros ========
ifneq (,$(wildcard $(XDC_INSTALL_DIR)/bin/echo.exe))
    # use these on Windows
    CP      = $(XDC_INSTALL_DIR)/bin/cp
    ECHO    = $(XDC_INSTALL_DIR)/bin/echo
    MKDIR   = $(XDC_INSTALL_DIR)/bin/mkdir -p
    RM      = $(XDC_INSTALL_DIR)/bin/rm -f
    RMDIR   = $(XDC_INSTALL_DIR)/bin/rm -rf
else
    # use these on Linux
    CP      = cp
    ECHO    = echo
    MKDIR   = mkdir -p
    RM      = rm -f
    RMDIR   = rm -rf
endif

#  ======== create output directories ========
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(PROFILE))
ifeq (,$(wildcard bin/$(PROFILE)))
    $(shell $(MKDIR) -p bin/$(PROFILE))
endif
endif
endif

