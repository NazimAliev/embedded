#!/usr/bin/python
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# rad2d.py
'''
radar antenna visual detection
based on fft2 spectrum peak of regular antenna array elements
'''
 
# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

# get "drone camera" picture 
PATH = '/data/rad2d/a%d.png'
sc = cv2.imread(PATH % 0, cv2.IMREAD_GRAYSCALE)
iy,ix = sc.shape
print(ix,iy)

shy = 130 
startx, endx = 70,ix 
out_1d = np.zeros((5,endx - startx))
lab = "Front View", "30 Deg Rot", "50 Deg Rot", "For", "Five"
col = 'b', 'g', 'r', 'b','b' 
'''
drawing input signal
'''
plt.figure(0)
for i in range(3):
	scene = cv2.imread(PATH % i, cv2.IMREAD_GRAYSCALE)
	out_1d[i] = scene[shy,startx:endx]
	plt.subplot("13%d" % (i+1))
	plt.imshow(cv2.cvtColor(scene,cv2.COLOR_GRAY2RGB),cmap = 'gray')
	plt.plot(np.arange(startx,endx,1),shy*np.ones(endx-startx), color = col[i], label = u'%s' % lab[i])
	plt.legend()

	plt.title(u'Drone camera picture'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

plt.show()
'''
drawing FFT(X)
'''
plt.figure(1)
for i in range(3):

	out_1d_abs = np.abs(np.fft.fftshift(np.fft.fft(out_1d[i] - np.mean(out_1d[i]))))
	out_1d_max = np.amax(out_1d_abs)
	OUT_1D = np.log10(out_1d_abs/out_1d_max + 1.0)
	plt.plot(OUT_1D[250:ix-320], label = u'%s' % lab[i])
	#plt.plot(np.arange(endx - startx),out_1d[i], label = u'%s' % lab[i])

plt.legend()
plt.title(u'Rotated Antenna X-spectrum'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')
plt.show()
