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
from scipy.signal import find_peaks_cwt
from matplotlib import pyplot as plt

# get "drone camera" picture 
PATH = '/data/rad2d/'

#scene = cv2.imread(PATH+'radar2.png', cv2.IMREAD_GRAYSCALE)
#scene = cv2.imread(PATH+'strips30.png', cv2.IMREAD_GRAYSCALE)
scene = cv2.imread(PATH+'asr0.png', cv2.IMREAD_GRAYSCALE)

iy,ix = scene.shape
scene_color = cv2.cvtColor(scene,cv2.COLOR_GRAY2RGB)

'''
ATTENTION! cv2 matrix has [Y,X] shape
'''
print(ix,iy)

# 2D FFT
SCENE = np.fft.fft2(scene-np.mean(scene))
out_abs = np.abs(SCENE - np.mean(SCENE))
out_max = np.amax(out_abs)
out_log = np.log10(out_abs/out_max + 1.0)
OUT_LOG = np.fft.fftshift(out_log)

# find FFT2 local maximums
peaks_x = np.array(find_peaks_cwt(OUT_LOG[iy/2,ix/2:], np.arange(1,28)))
peaks_y = np.array(find_peaks_cwt(OUT_LOG[iy/2:,ix/2], np.arange(1,6)))
print("X freqs:")
print(peaks_x)
print("X period:")
print(ix/peaks_x)
print("Y freqs:")
print(peaks_y)
print("Y period:")
print(iy/peaks_y)

# picture section x,y coordinates for 1D FFT
#shx = 125 
#shy = 45 

# for ASR
#shx = 250 
#shy = 220 

# for another radar picture 
shx = ix/2 
shy = 220 
shy2 = 355 

startx, endx = 10,570 

'''
drawing input signal
'''

plt.subplot(121)
plt.imshow(scene_color,cmap = 'gray')
# cross lines
plt.plot(shx*np.ones(iy),np.arange(iy),'g')
#plt.plot(np.arange(ix),shy*np.ones(ix),'b')
#plt.plot(np.arange(ix),shy2*np.ones(ix),'b')
plt.plot(np.arange(startx,endx,1),shy*np.ones(endx-startx),'b')
plt.plot(np.arange(startx,endx,1),shy2*np.ones(endx-startx),'r')
plt.title(u'Drone camera picture')
plt.xlabel(u'(c) Nazim.RU')

'''
drawing FFT2
'''

sp = plt.subplot(122)
# set symmetry axis labels with zero in center
sp.set_xticks(np.arange(0,ix,ix/10))
sp.set_xticklabels(np.linspace(-ix/2,ix/2,11).astype(int))
sp.set_yticks(np.arange(0,iy,iy/6))
sp.set_yticklabels(np.linspace(-iy/2,iy/2,7).astype(int))

# show peaks to test match with local maxs
'''
plt.plot(ix/2 + peaks_x, iy*np.ones(len(peaks_x))/2 - 4, 'o')
plt.plot(4+ix*np.ones(len(peaks_y))/2, iy/2 + peaks_y, 'o')
'''

cut = 0
#plt.imshow(OUT_LOG[cut:iy-cut,cut:ix-cut],cmap='gray')
# to remove bright central peak
clip = np.minimum(OUT_LOG, 0.05)
plt.imshow(clip,cmap='gray')
# cross - lines
#plt.plot(np.arange(ix-2*cut),(iy-2*cut)*np.ones(ix-2*cut)/2, 'g')
#plt.plot((ix-2*cut)*np.ones(iy-2*cut)/2, np.arange(iy-2*cut), 'g')
#plt.title(u'2D Spectrum'), plt.xticks([]), plt.yticks([])
plt.title(u'2D Spectrum')
plt.xlabel(u'(c) Nazim.RU')
plt.show()

'''
drawing signal X 
'''

plt.subplot(221)
#out_1d = scene[shy,:]
out_1d = scene[shy,startx:endx]
out_1d2 = scene[shy2,startx:endx]
# FIXME
#t = np.arange(ix)
#out_1d = np.sin(2*np.pi*t*5/ix)
plt.plot(out_1d, 'b')
plt.plot(out_1d2, 'r')
plt.title(u'X-section'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

'''
drawing FFT(X)
'''

plt.subplot(222)
out_1d_abs = np.abs(np.fft.fftshift(np.fft.fft(out_1d - np.mean(out_1d))))
out_1d_max = np.amax(out_1d_abs)
OUT_1D = np.log10(out_1d_abs/out_1d_max + 1.0)
out_1d_abs2 = np.abs(np.fft.fftshift(np.fft.fft(out_1d2 - np.mean(out_1d2))))
out_1d_max2 = np.amax(out_1d_abs2)
OUT_1D2 = np.log10(out_1d_abs2/out_1d_max2 + 1.0)
#plt.plot(OUT_1D[ix/4:3*ix/4], 'b')
plt.plot(OUT_1D, 'b')
plt.plot(OUT_1D2, 'r')
plt.title(u'X-spectrum'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

'''
drawing signal Y 
'''

plt.subplot(223)
#out_1d = scene[81:185,shx]
out_1d = scene[:,shx]
plt.plot(out_1d, 'g')
plt.title(u'Y-section'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

'''
drawing FFT(Y)
'''

plt.subplot(224)
out_1d_abs = np.abs(np.fft.fftshift(np.fft.fft(out_1d - np.mean(out_1d))))
out_1d_max = np.amax(out_1d_abs)
OUT_1D = np.log10(out_1d_abs/out_1d_max + 1.0)
plt.plot(OUT_1D[iy/4:3*iy/4], 'g')
#plt.plot(OUT_1D, 'g')
plt.title(u'Y-spectrum'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

plt.show()
