#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

#
#  ======== makefile ========
#  GNUmake-based makefile.
#

.PHONY: all

all:
	$(MAKE) -C  codec_dsp
	$(MAKE) -C  server_dsp
	$(MAKE) -C  main_app

install:
	$(MAKE) -C  main_app install
	$(MAKE) -C  server_dsp install

clean::
	$(MAKE) -C  main_app $@
	$(MAKE) -C  codec_dsp $@
	$(MAKE) -C  server_dsp $@
