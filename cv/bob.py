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

i0 = 0
i1 = 255

def layer(img, N, n, s1, s2):
    N2 = int(N/2)
    img2 = np.zeros([N2,N2],dtype=np.uint8)
    img2.fill(i0)
    for row in range(0,N-1,2):
        for col in range(0,N-1,2):
            if img[row,col]==i1 & img[row,col+1]==i1 & img[row+1,col]==i1 & img[row+1,col+1]==i1:
                print(row,col)
                img2[int(row/2),int(col/2)]=i1
                img[row,col]=i0
                img[row,col+1]=i0
                img[row+1,col]=i0
                img[row+1,col+1]=i0
                
    ax2 = fig.add_subplot(2, 4, n)
    ax2.imshow(img2,cmap = 'gray')
    ax2.set_xticks(range(int(N/2)))
    ax2.set_yticks(range(int(N/2)))
    ax2.grid()
    plt.title(s1)

    ax3 = fig.add_subplot(2, 4, n+1)
    ax3.imshow(img,cmap = 'gray')
    ax3.set_xticks(range(int(N)))
    ax3.set_yticks(range(int(N)))
    ax3.grid()
    plt.title(s2)
    return img2

N = 32
size = (N,N)

img = np.zeros([N,N],dtype=np.uint8)
img.fill(i0)

img[0:4,20:21] = i1
# 1/32
img[0:2,16:20] = i1
# 1/16
img[28:32,16:20] = i1
# 1/8
img[:16,8:16] = i1
# 1/4
img[16:,:16] = i1

fig = plt.figure()

#####
img2 = layer(img,N, 1, "1st proc", "A layer")
img3 = layer(img2,int(N/2), 3, "2nd proc", "B layer")
img4 = layer(img3,int(N/4), 5, "3th proc", "C layer")
img5 = layer(img4,int(N/8), 7, "4th proc", "D layer")

plt.show()
plt.close()
