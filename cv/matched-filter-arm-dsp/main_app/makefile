
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

NE10_LIB=$(BOARD_SUPPORT)/ne10/build/modules/libNE10.a

#
#  ======== define all the repositories needed by this app ========
#

# root dir of our source system
ROOT_DIR ?= $(DSP_SUPPORT)/../

XDCTOOLS_REPO   = $(XDC_INSTALL_DIR)/packages
CE_REPO         = $(CE_INSTALL_DIR)/packages
SUFFIX          = v7A

#
#  Make sure CE_INSTALL_DIR was set properly. By using "-include" to include
#  xdcpaths.mak, no error message will be given if xdcpaths.mak is not found,
#  and in that case, the user must have set the environment variables, for
#  example, on the command line.
# 
ERRMSG = which is invalid (could not find file "$(TEST_FILE)"). Set path in products.mak file! See the build documentation to correct this error.
TEST_FILE := $(CE_REPO)/ti/sdo/ce/package.xdc
ifneq (clean,$(MAKECMDGOALS))
ifeq ($(wildcard $(TEST_FILE)),)
    $(error CE_INSTALL_DIR is set to "$(CE_INSTALL_DIR)", $(ERRMSG))
endif
endif

REPO_PATH = $(XDCTOOLS_REPO) $(CE_REPO) $(LINK_INSTALL_DIR)/packages $(XDAIS_INSTALL_DIR)/packages $(FC_INSTALL_DIR)/packages \
	$(CMEM_INSTALL_DIR)/packages

#
#  ======== common defines used by TI build system========
#
ALL.defs =
ALL.incs = -I. $(addprefix -I,$(REPO_PATH))
ALL.libs = $(addprefix -L,$(REPO_PATH))

EXTLIBS_$(PROFILE) = osal/$(PROFILE)/lib/osal.a osal/$(PROFILE)/lib/cstubs.a

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
CPP = $(CGTOOLS_V7A)/$(CPP_V7A)

CCPROFILE.debug = -g -D_DEBUG_=1 -DNPROFILE=1
CCPROFILE.release = -O2 -DNPROFILE=0

CFLAGS = -fPIC -Wall -Werror -mfpu=neon -fno-strict-aliasing $(CCPROFILE.$(PROFILE)) \
    -I. $(addprefix -I,$(REPO_PATH))

CPPFLAGS = -Dxdc_target_name__=GCArm$(SUFFIX) \
    -Dxdc_target_types__=gnu/targets/arm/std.h

LNKPROFILE.debug = -g
LNKPROFILE.release =
LDFLAGS = $(LNKPROFILE.$(PROFILE))
LDLIBS = -lpthread -lrt -ldl

#
#  This app is built with the following source files:
#
#  $(CE_REPO)/ti/sdo/ce/utils/rtcfg/
#      rtcfg.c
#      rtcfg_fcinit.c	
#      rtcfg_local_config.c
#
#      The source files in $(CE_REPO)/ti/sdo/ce/utils/rtcfg are run-time
#      configuration files.  These files do not need to be modified to run this
#      app. If you want to modify the configuration in any of these files,
#      copy it to this directory and modify as needed. The vpath variable
#      below sets the search path for these source files.

#      The file, rtcfg_remote_config.c, contains application and device
#      specific configuration.  This file is located in
#      <CE_INSTALL_DIR>/packages/ti/sdo/ce/utils/rtcfg, and is located by
#      make through the vpath variable set below.  If needed, this file can
#      be copied to this directroy and modified.
#
#
#  The vpath variable lets us maintain some of the same sources for both XDC
#  build and gmake build, and use default run-time configuration files.
#
vpath %.c $(CE_REPO)/ti/sdo/ce/utils/rtcfg

APP.srcs_c = $(wildcard ./*.c) rtcfg.c rtcfg_fcinit.c rtcfg_remote_config.c   

APP.srcs_cpp =  $(wildcard ./*.cpp)

APP.srcs = $(APP.srcs_cpp) $(APP.srcs_c) 

APP.objs_c = $(addprefix bin/$(PROFILE)/, $(patsubst %.c,%.o$(SUFFIX),$(APP.srcs_c)))

APP.objs_cpp = $(addprefix bin/$(PROFILE)/, $(patsubst %.cpp,%.ocpp$(SUFFIX),$(APP.srcs_cpp)))
	   
APP.objs = $(APP.objs_cpp) $(APP.objs_c) 

APP.headers = $(patsubst %.c,%.h,$(APP.srcs_c)) \
	          $(patsubst %.cpp,%.h,$(APP.srcs_cpp))

#
#  ======== linker command files ========
#
#  Set up path to find linker command files, and which linker command
#  files will be used (eg, debug or release).
#

#  Path for linker command files. We use several linker command files,
#  keeping those containing Codec Engine and Framework Components libraries
#  in a common area where they can be used by multiple apps. The linker
#  command files in this directory just contain the codec libraries needed
#  by this app.
#
#  The common linker command files, in ti/sdo/ce/utils/rtcfg/linux/OMAP3530,
#  shouldn't need modification, but in the chance that they did, they could be
#  copied to this directory and modified as needed. The vpath will cause
#  any modified linker command files to be found first.
#
vpath %.cmd $(CE_REPO)/ti/sdo/ce/utils/rtcfg/linux/OMAP3530

# Codec Engine libraries
CE_LINKER_FILE = ce_remote_$(PROFILE).cmd

# Framework Components libraries
FC_LINKER_FILE = fc_$(PROFILE).cmd

PROGRAM = af_app

.PHONY: all install clean

#
#  ======== make commands ========
#
all:
	@$(ECHO) "# [1] Making all $@ ..."
	#@touch app.cpp
	@$(MAKE) bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX)


#
#  ======== build rules ========
#
bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX): $(APP.objs) $(EXTLIBS_$(PROFILE)) $(CE_LINKER_FILE) $(FC_LINKER_FILE) 
	@$(ECHO) "# [2] Making bin/profile/program $@ ..."
# always recompile app.cpp to get correct value of __TIME__ and __DATE__ macros
	#@touch app.cpp
	@$(CPP) -o $@ $^ $(EXTLIBS_$(PROFILE)) $(LDLIBS) $(ALL.defs) $(ALL.libs) $(NE10_LIB)

.SECONDARY:

bin/$(PROFILE)/%.ocpp$(SUFFIX): %.cpp
	@$(ECHO) "# [3] Making bin/profile/ocpp $@ ..."
	@$(CPP) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
	@$(CPP) $(CPPFLAGS) $(CFLAGS) -MM -MT $@ -c $< > $@.d

bin/$(PROFILE)/%.o$(SUFFIX): %.c
	@$(ECHO) "# [4] Making bin/profile/o $@ ..."
	@$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MM -MT $@ -c $< > $@.d

# including header dependencies
-include $(APP.objs_cpp:.ocpp$(SUFFIX)=.ocpp$(SUFFIX).d)
-include $(APP.objs_c:.o$(SUFFIX)=.o$(SUFFIX).d)


#
# OSAL
#
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(PROFILE))
ifeq (,$(wildcard $(PROFILE)))
    $(shell $(MKDIR) -p osal)
    $(shell $(MKDIR) -p osal/$(PROFILE))
endif
endif
endif

CROSS_COMPILE = $(subst -gcc,-,$(CGTOOLS_V7A)/$(CC_V7A))

osal/debug/lib/osal.a:
	@echo "[5] Making osal..."
	@$(MAKE) -C osal/debug \
            -f $(OSAL_INSTALL_DIR)/packages/linuxdist/build/Makefile \
            -s \
            OSAL_INSTALL_DIR=$(OSAL_INSTALL_DIR) \
            CROSS_COMPILE=$(CROSS_COMPILE) \
            PRECONFIG=$(OSAL_INSTALL_DIR)/packages/linuxdist/preconfig/development \
            CFLAGS=-g XDC_TARGET=$(XDC_TARGET) SUFFIX=$(SUFFIX)

osal/debug/lib/cstubs.a:
	@echo "[6] Making cstubs..."
	@$(MAKE) -s -C osal/debug \
            -f $(OSAL_INSTALL_DIR)/packages/linuxdist/cstubs/Makefile \
            -s \
            OSAL_INSTALL_DIR=$(OSAL_INSTALL_DIR) \
            CROSS_COMPILE=$(CROSS_COMPILE) \
            CFLAGS=-g XDC_TARGET=$(XDC_TARGET) SUFFIX=$(SUFFIX)

osal/release/lib/osal.a:
	@echo "[7] Making osal..."
	@$(MAKE) -C osal/release \
            -f $(OSAL_INSTALL_DIR)/packages/linuxdist/build/Makefile \
            -s \
            OSAL_INSTALL_DIR=$(OSAL_INSTALL_DIR) \
            CROSS_COMPILE=$(CROSS_COMPILE) \
            PRECONFIG=$(OSAL_INSTALL_DIR)/packages/linuxdist/preconfig/production \
            CFLAGS=-O2 XDC_TARGET=$(XDC_TARGET) SUFFIX=$(SUFFIX)

osal/release/lib/cstubs.a:
	$(MAKE) -C osal/release \
            -f $(OSAL_INSTALL_DIR)/packages/linuxdist/cstubs/Makefile \
            -s \
            OSAL_INSTALL_DIR=$(OSAL_INSTALL_DIR) \
            CROSS_COMPILE=$(CROSS_COMPILE) \
            CFLAGS=-O2 XDC_TARGET=$(XDC_TARGET) SUFFIX=$(SUFFIX)

install:
	@$(ECHO) "# [8] Making $@ ..."
	scp bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX) root@bb2:vbls/
	#cp bin/$(PROFILE)/$(PROGRAM).x$(SUFFIX) $(EXBASE)/../../../export/vbls
	
#
#  ======== clean ========
#
clean::
	$(RMDIR) bin
	$(RMDIR) osal

