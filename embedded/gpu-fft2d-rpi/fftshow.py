#!/usr/bin/python3
# -*- coding: utf8 -*-


#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

import os,sys
import numpy as np
import codecs
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg

if len(sys.argv) !=3:
	print("Usage: fftshow N jobs")
	exit(0)

N = int(sys.argv[1])
jobs = int(sys.argv[2])

app = QtGui.QApplication([])
win = pg.GraphicsWindow(title="Basic plotting examples")
win.resize(1000,600)
win.setWindowTitle('FFT Based On GPU')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)

#UTF8Reader = codecs.getreader('utf8')
#sys.stdin = UTF8Reader(sys.stdin)

dt = np.dtype('float32')
buf = sys.stdin.buffer.read()
data = np.frombuffer(buf, dtype=dt)/1000.0
fft = np.hypot(data[0::2],data[1::2])
fft = fft[0:N*jobs]
fft = fft.reshape(N, jobs)

p6 = win.addPlot(title="FFT")
curve = p6.plot(pen='y')
curve2 = p6.plot(pen='r')
ptr = 0

def update():
	global curve, curve2,vfft, ptr, jobs
	data = np.log10(fft[ptr,:]+1.0)
	data2 = np.log10(fft[:,ptr]+1.0)
	curve.setData(data)
	curve2.setData(data2)
	ptr += 1
	if ptr == jobs:
		ptr = 0

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(20)


## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()

