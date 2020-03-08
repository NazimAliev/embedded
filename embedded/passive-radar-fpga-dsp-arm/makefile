# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

#
#  ======== makefile ========
#  GNUmake-based makefile.
#

.PHONY: all

all:
	$(MAKE) -C  dsp 
	$(MAKE) -C  zboard 
	$(MAKE) -C  multix86 

install:
	$(MAKE) -C  dsp install 
	$(MAKE) -C  zboard install

clean::
	$(MAKE) -C  dsp $@
	$(MAKE) -C  zboard $@
	$(MAKE) -C  multix86 $@
