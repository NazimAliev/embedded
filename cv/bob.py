#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# bob.py
# encoder to signature E2S 

# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

N = 8
N2 = 4
size = (N,N)
i0 = 0 
i1 = 255

img = np.zeros([N,N],dtype=np.uint8)
img.fill(i0)
img[2:6,0:2] = i1
img[:6,2:4] = i1
img[2:6,4:6] = i1
img[6:,0] = i1
img[6:,5] = i1
img[2,6] = i1

plt.figure(1)
plt.imshow(img,cmap = 'gray')
plt.title('Bob image'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

plt.show()
plt.close()

img2 = np.zeros([N2,N2],dtype=np.uint8)
img2.fill(i0)
for row in range(0,N-1,2):
    for col in range(0,N-1,2):
        print(row,col)
        if img[row,col]==i1 & img[row,col+1]==i1 & img[row+1,col]==i1 & img[row+1,col+1]==i1:
            img2[int(row/2),int(col/2)]=i1
            img[row,col]=i0
            img[row,col+1]=i0
            img[row+1,col]=i0
            img[row+1,col+1]=i0

plt.figure(2)
plt.imshow(img2,cmap = 'gray')
plt.title('Bob image'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

plt.figure(3)
plt.imshow(img,cmap = 'gray')
plt.title('Bob image'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

plt.show()
plt.close()
