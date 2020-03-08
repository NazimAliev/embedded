#!/usr/bin/python
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# fnoise.py
# test fractal marker for VBLS

# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

import plot3d

scale = 5.0 
N = 8
size = (N,N)

PATH='/data/'
# load picture to place marker in
scene_img = cv2.imread(PATH+'scene.jpg', cv2.IMREAD_GRAYSCALE)

marker_img = cv2.imread(PATH+'unit1/pif0.png', cv2.IMREAD_GRAYSCALE)
# FIXME
marker_img = cv2.resize(marker_img,None,fx=0.5, fy=0.5, interpolation = cv2.INTER_CUBIC)

# marker to put on scene, testing with different sizes
(m_x, m_y) = marker_img.shape
scene_img[50:50+m_x,50:50+m_y] = marker_img


# resize scene using scale to test matching 
resized_scene_img = cv2.resize(scene_img,None,fx=scale, fy=scale, interpolation = cv2.INTER_CUBIC)
img = resized_scene_img.copy()

# load at first to rescue memory
template_img = cv2.imread(PATH+'unit1/pif0.png', cv2.IMREAD_GRAYSCALE)

max_val = 0
max_i = 0
for i in range(8):
    template_img = cv2.imread(PATH+'unit1/pif%d.png' % i, cv2.IMREAD_GRAYSCALE)
    # FIXME
    template_img = cv2.resize(template_img,None,fx=0.5, fy=0.5, interpolation = cv2.INTER_CUBIC)

    # Apply template Matching
    # =======================
    res = cv2.matchTemplate(img,template_img,cv2.TM_CCOEFF)
    minval, maxval, minloc, maxloc = cv2.minMaxLoc(res)
    # =======================
    # save parms if mathcing is better
    if maxval > max_val:
        ## for log
        max_i = i
        # to find better matching
        max_val = maxval
        # for draw rectangle
        max_loc = maxloc
        # drawing
        max_res = res
        max_img = img
        max_template = template_img
print(max_i)

# If the method is TM_SQDIFF or TM_SQDIFF_NORMED, take minimum
top_left = max_loc
w, h = max_template.shape[::-1]
bottom_right = (top_left[0] + w, top_left[1] + h)

max_img_rgb = cv2.cvtColor(max_img,cv2.COLOR_GRAY2RGB)
cv2.rectangle(max_img_rgb,top_left, bottom_right, (10,255,10), 2)

plt.figure(1)
plt.imshow(max_res,cmap = 'gray')
plt.title('2D Marker Corr Function'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')
plt.figure(2)
plt.imshow(max_img_rgb)
plt.title('Detected Marker'), plt.xticks([]), plt.yticks([])
plt.xlabel(u'(c) Nazim.RU')

'''
plt.figure(3)
plt.imshow(img_N,interpolation='none',cmap='gray')
plt.title('VBLS Marker')
plt.xlabel(u'(c) Nazim.RU')
'''

plt.show()
plt.close()

plot3d.plot3d(8*max_res/max_val)

