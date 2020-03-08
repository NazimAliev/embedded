#!/usr/bin/python
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# filter.py

'''
2d image filtration on Mahjong cubes

scene: input signal, contains useful signal ref and background picture (noise)
ref: known reference signal, use as reference
filter: output filter

data processing follows convolve in frequency domain: matched filter

SIGNAL = FFF(scene)*FFT(ref).conj
a SIGNAL peak placement correspondes to place the ref on scene
'''
 
# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

import plot3d

# get reference picture
unit = cv2.imread('/data/mj-unit.png', cv2.IMREAD_GRAYSCALE)
ix,iy = unit.shape

#get scene picture
scene = cv2.imread('/data/mahjong.jpg', cv2.IMREAD_GRAYSCALE)
# ref template coordinates on scene
sx,sy = (50,300)
# put reference block on scene
scene[sx:sx+ix,sy:sy+iy] = unit 

# set ref dimensions as a scene
# scene and ref dims must be equal, fill white remain part of ref
ref = np.zeros(scene.shape,dtype=np.uint8)
ref[:,:] =255 
ref[0:ix,0:iy] = unit 

# FFTs
SCENE = np.fft.fft2(scene-np.mean(scene))
REF = np.fft.fft2(ref-np.mean(ref))

# ======== filtration ========
OUT = SCENE*np.conj(REF)
# ============================

out = np.fft.ifft2(OUT)

# normalization for plotting
#out_abs = np.abs(np.fft.ifftshift(out))
out_abs = np.abs(out)

out_m = np.amax(out_abs)
argmax = np.argmax(out_abs)
argmax2d = np.unravel_index(argmax, np.array(out_abs).shape)
print(argmax2d)
x,y = argmax2d
# drawing
plt.subplot(121)
scene_i = cv2.cvtColor(scene,cv2.COLOR_GRAY2RGB)
cv2.rectangle(scene_i,(y, x), (y+iy,x+ix), (10,255,10), 2)
plt.imshow(scene_i,cmap = 'gray')
plt.title(u'Сигнал'), plt.xticks([]), plt.yticks([])

plt.subplot(122)
out_log = np.log10(out_abs/out_m + 1.0)
plt.imshow(out_log,cmap='gray')
plt.title(u'Согласованный фильтр'), plt.xticks([]), plt.yticks([])

plt.show()

# ====================
plot3d.plot3d(out_abs)
# ====================
