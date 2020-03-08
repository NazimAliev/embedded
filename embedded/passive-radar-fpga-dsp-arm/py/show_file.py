#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import os,sys
import numpy as np
import matplotlib.pyplot as plt

filename1 = '/data/nefilter1.bin'
filename2 = '/data/nefilter2.bin'

# >i4 32bit int, big endian
# <i4 32bit int, little endian

raw1 = np.fromfile(filename1, dtype=np.float32)
raw2 = np.fromfile(filename2, dtype=np.float32)
print(raw1[:16])
print(raw2[:16])

frame = 1024/4 

fig = plt.figure(1)

ax1 = fig.add_subplot(211)
ax1.plot(raw1[:frame], label='1st')
ax1.legend()

ax2 = fig.add_subplot(212)
ax2.plot(raw2[:frame], label='2nd')
ax2.legend()

plt.show()
