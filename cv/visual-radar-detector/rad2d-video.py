#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# rad2d-video.py

'''
fft2 of rotated radar antenna, video version
'''
 
# import the necessary packages
import cv2
import os,sys
import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
from time import sleep

video = '/data/rad2d/rad.ogg'
camera = cv2.VideoCapture(video)
frame_w = int(camera.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_h = int(camera.get(cv2.CAP_PROP_FRAME_HEIGHT))
# frame size
print("Video size: %dx%d" % (frame_w,frame_h))

#QtGui.QApplication.setGraphicsSystem('raster')
app = QtGui.QApplication([])
#mw = QtGui.QMainWindow()
#mw.resize(800,800)

win = pg.GraphicsWindow(title="Basic plotting examples")
win.resize(1000,600)
win.setWindowTitle('1D Visual Rotated Radar Antenna Spectrum')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)
p1 = win.addPlot(title="Top Antenna FFT", row=0, col=0)
p2 = win.addPlot(title="Bottom Antenna FFT", row=1, col=0)
curve1 = p1.plot(pen='y')
curve2 = p2.plot(pen='r')

win2 = pg.GraphicsLayoutWidget()
win2.show()  ## show widget alone in its own window
win2.setWindowTitle('Rotated Radar Antenna Camera Image')
view = win2.addViewBox()
img = pg.ImageItem(border='w')
view.addItem(img)


lx,ly = 300,300
ptr = 0

'''
fft2 and drawing oscillogram
data: radar antenna image
lvl: horizontal slice y-level to get 1D data
blank: supress strong fft peaks near zero frequency
win: fft wight
k: amplitude factor
shift: move graph up to better vision
'''
def proc(data, lvl, curve, blank, win, k=1.0, shift=0):

	d = np.sum(data[:,lvl-5:lvl+5], axis=1)
	d = d - np.mean(d)
	#f = np.abs(np.fft.fftshift(np.fft.fft(d)))
	f = np.abs(np.fft.fft(d))
	fmax = np.amax(f)
	F = np.log10(f/fmax + 1.0) + shift
	sz = F.size
	#F[sz//2-blank:sz//2+blank] = 0 
	F[:blank] = shift 
	curve.setData(np.arange(win),k*F[:win])

i = 0
while(1):
	(grabbed, fread) = camera.read()
	if not grabbed:
		camera.set(cv2.CAP_PROP_POS_FRAMES, 0)
		continue
		#break

	frame_read = fread[frame_h//2-ly:frame_h//2+ly,frame_w//2-lx:frame_w//2+lx]
	frame = cv2.cvtColor(frame_read, cv2.COLOR_BGR2GRAY)

	rad = np.fliplr(frame.T.astype('float64'))
	lvl, lvl2 = 431,231	
	fftwin = 150
	'''*****************************************'''
	proc(rad, lvl, curve1, 20, 2*fftwin, k=2.5, shift=0.0)
	proc(rad, lvl2, curve2, 20, 2*fftwin, k=6.0, shift=0.0)
	'''*****************************************'''

	rad[:,lvl-1:lvl+1] = 0 
	rad[:,lvl2-1:lvl2+1] = 0 
	img.setImage(rad)

	if ptr == 0:
		curve1.setData(np.arange(fftwin),0.5*np.ones(fftwin))
		curve2.setData(np.arange(fftwin),0.5*np.ones(fftwin))
		p1.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted
		p2.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted

	key = cv2.waitKey(0)
	if key & 0xFF == 27:
		break
	if key == 32:
		print("Ops")
	sleep(0.01)

	ptr += 1

camera.release()
# cleanup the camera and close any open windows
cv2.destroyAllWindows()

