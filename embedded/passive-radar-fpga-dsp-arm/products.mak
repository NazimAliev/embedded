# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

# Optional, but useful when many/all dependent components are in one folder
#
DEPOT = /home/legolas/bin/bbxm/dsp-support
BOARD = /home/legolas/bin/bbxm/board-support

# Set this to your CE installation dir
CE_INSTALL_DIR = $(DEPOT)/codec_engine_3_23_00_07/

#PROFILE = debug
PROFILE = debug 

# Define the product variables for the device you will be using.
#IPC_INSTALL_DIR         = $(DEPOT)/ipc_1_25_01_09
FC_INSTALL_DIR          = $(CE_INSTALL_DIR)/cetools
#FC_INSTALL_DIR          = $(DEPOT)/framework_components_3_31_00_02
LINK_INSTALL_DIR        = $(DEPOT)/syslink_2_21_00_03
OSAL_INSTALL_DIR        = $(CE_INSTALL_DIR)/cetools
#OSAL_INSTALL_DIR        = $(DEPOT)/osal_1_24_00_09
XDAIS_INSTALL_DIR       = $(CE_INSTALL_DIR)/cetools/
#XDAIS_INSTALL_DIR       = $(DEPOT)/xdais_7_24_00_04
CMEM_INSTALL_DIR        = $(DEPOT)/linuxutils_3_23_00_01
EDMA3_LLD_INSTALL_DIR   = $(CE_INSTALL_DIR)/cetools
#EDMA3_LLD_INSTALL_DIR   = $(DEPOT)/edma3_lld_02_12_00_20
XDC_INSTALL_DIR         = $(DEPOT)/xdctools_3_24_05_48
BIOS_INSTALL_DIR        = $(DEPOT)/bios_6_34_03_19
DSPLIB_INSTALL_DIR      = $(DEPOT)/dsplib_3_4_0_0
NE10_INSTALL_DIR      = $(BOARD)/ne10

# Set location of various cgtools
ti.targets.C64P =
ti.targets.C674 =

ti.targets.elf.C64P = $(DEPOT)/cgt6x_7_3_1/
ti.targets.elf.C64T =
ti.targets.elf.C66 =
ti.targets.elf.C674 =

ti.targets.arm.elf.M3 =

# Note that GCC targets are 'special' and require more than one var to be set.
#
# The CGTOOLS_* var points at the base of the toolchain.
# The CC_* var points at the gcc binary (e.g. bin/arm-none-linux-gnueabi-gcc)
#
# XXX
#CGTOOLS_V5T = $(DEPOT)/../linux-devkit-hf
#CC_V5T      = bin/arm-linux-gnueabihf-gcc
#CPP_V5T      = bin/arm-linux-gnueabihf-g++

XDC_TARGET = GCArmv7A
#CGTOOLS_V7A = $(DEPOT)/../linux-devkit-hf
CGTOOLS_V7A = /usr
CC_V7A      = bin/arm-linux-gnueabihf-gcc
CPP_V7A      = bin/arm-linux-gnueabihf-g++

#CGTOOLS_V5T = $(DEPOT)/../linux-devkit
#CC_V5T      = bin/arm-arago-linux-gnueabi-gcc 
#CPP_V5T      = bin/arm-arago-linux-gnueabi-g++

# The AR_* var points at the ar binary (e.g. bin/arm-none-linux-gnueabi-ar)
# We can often auto-determine this based on the value of CC_V5T.
# The magic make cmd replaces the "-gcc" at the end of CC_V5T var with "-ar".
#AR_V5T      = $(CC_V5T:-gcc=-ar)
AR_V7A      = $(CC_V7A:-gcc=-ar)

# don't modify this, it's derived from the GCC vars above
#gnu.targets.arm.GCArmv5T = $(CGTOOLS_V5T);LONGNAME=$(CC_V5T);profiles.release.compileOpts.copts=-O2 -ffunction-sections
gnu.targets.arm.GCArmv7A = $(CGTOOLS_V7A);LONGNAME=$(CC_V7A);profiles.release.compileOpts.copts=-O2 -ffunction-sections

# Use this goal to print your product variables.
.show-products::
#	@echo "DEPOT                       = $(DEPOT)"
	@echo "LINK_INSTALL_DIR            = $(LINK_INSTALL_DIR)"
	@echo "CMEM_INSTALL_DIR            = $(CMEM_INSTALL_DIR)"
	@echo "XDAIS_INSTALL_DIR           = $(XDAIS_INSTALL_DIR)"
	@echo "IPC_INSTALL_DIR             = $(IPC_INSTALL_DIR)"
	@echo "EDMA3_LLD_INSTALL_DIR       = $(EDMA3_LLD_INSTALL_DIR)"
	@echo "FC_INSTALL_DIR              = $(FC_INSTALL_DIR)"
	@echo "OSAL_INSTALL_DIR            = $(OSAL_INSTALL_DIR)"
	@echo "CGTOOLS_V7A                 = $(CGTOOLS_V7A)"
	@echo "CC_V7A                      = $(CC_V7A)"
	@echo "AR_V7A                      = $(AR_V7A)"
	@echo "ti.targets.C64P      (COFF) = $(ti.targets.C64P)"
	@echo "ti.targets.C674      (COFF) = $(ti.targets.C674)"
	@echo "ti.targets.elf.C64T   (ELF) = $(ti.targets.elf.C64T)"
	@echo "ti.targets.elf.C64P   (ELF) = $(ti.targets.elf.C64P)"
	@echo "ti.targets.elf.C674   (ELF) = $(ti.targets.elf.C674)"
	@echo "ti.targets.elf.C66    (ELF) = $(ti.targets.elf.C66)"
	@echo "ti.targets.arm.elf.M3 (ELF) = $(ti.targets.arm.elf.M3)"
	@echo "gnu.targets.arm.GCArmv7A    = $(gnu.targets.arm.GCArmv7A)"
	@echo "PROFILE                     = $(PROFILE)"


