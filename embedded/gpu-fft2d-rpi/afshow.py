#!/usr/bin/python
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
import pyqtgraph.opengl as gl

if len(sys.argv) !=2:
	print("Usage: afshow x-samples")
	exit(0)

# number of FFT samples
x_samples = int(sys.argv[1])

## Create a GL View widget to display data
app = QtGui.QApplication([])
w = gl.GLViewWidget()
w.show()
w.setWindowTitle('Ambiguity Function (c) Nazim.RU')
w.setCameraPosition(distance=50)

## Add a grid to the view
g = gl.GLGridItem()
g.scale(2,2,1)
g.setDepthValue(10)  # draw grid after surfaces since they may be translucent
w.addItem(g)


## Animated example
## compute surface vertex data
x = np.linspace(-8, 8, x_samples).reshape(x_samples,1)
y = np.linspace(-4, 4, x_samples).reshape(1,x_samples)

p4 = gl.GLSurfacePlotItem(x=x[:,0], y = y[0,:], shader='heightColor', computeNormals=False, smooth=False)
p4.shader()['colorMap'] = np.array([0.2, 2, 0.5, 0.2, 1, 1, 0.2, 0, 2])
# сместить к краю
#p4.translate(10, 10, 0)
p4.scale(1,2,4)
w.addItem(p4)

# expects 4-byte float re, im numbers
dt = np.dtype('float32')

# for python2
#buf = sys.stdin.buffer.read()
buf = sys.stdin.read()
data = np.frombuffer(buf, dtype=dt)

# from complex floats to abs
cx = np.abs(data[0::2] + 1j*data[1::2])
af = np.log10(cx+1.0) - np.log10(1.0)
z = af.reshape(x_samples, x_samples/2)
Z = np.zeros((x_samples,x_samples))
Z[:,0:x_samples/2] = z
#Z[:,x_samples/2:x_samples] = 0

index = 0
def update():
	global p4, index, Z 
	index += 0.1
	p4.setData(z=Z)

# timer loop
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(30)


## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()

