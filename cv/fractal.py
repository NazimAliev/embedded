#!/usr/bin/python
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

from pyqtgraph.Qt import QtCore, QtGui
import pyqtgraph.opengl as gl
import pyqtgraph as pg

'''
fractal.py

find unit on Pifagor tree
'''
PATH='/data/'
scale = 1.9 

def fixResize(img):
	resized_img = cv2.resize(img,None,fx=0.25, fy=0.25, interpolation = cv2.INTER_NEAREST)
	return resized_img.copy()

# get fractal picture
fractal_img = cv2.imread(PATH+'pif.png', cv2.IMREAD_GRAYSCALE)
fractal_img = fixResize(fractal_img)
resised_img = cv2.resize(fractal_img,None,fx=scale, fy=scale, interpolation = cv2.INTER_LINEAR)
img = resised_img.copy()

max_val = 0
max_i = 0
for i in range(8):
	template_img = cv2.imread(PATH+'unit2/pif%d.png' % i, cv2.IMREAD_GRAYSCALE)
	template_img = fixResize(template_img)

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

# drow restangling around matching place
w, h = max_template.shape[::-1]
top_left = max_loc
bottom_right = (top_left[0] + w, top_left[1] + h)
max_img_rgb = cv2.cvtColor(max_img,cv2.COLOR_GRAY2RGB)
cv2.rectangle(max_img_rgb,top_left, bottom_right, (10,255,10), 2)

# draw res and template
plt.subplot(121),plt.imshow(max_res,cmap = 'gray')
plt.title('Matching Result'), plt.xticks([]), plt.yticks([])
plt.subplot(122),plt.imshow(max_img_rgb)
plt.title('Detected Point'), plt.xticks([]), plt.yticks([])

plt.show()

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
(sh_x, sh_y) = max_res.shape
x = np.linspace(-8, 8, sh_x).reshape(sh_x,1)
y = np.linspace(-8, 8, sh_y).reshape(1,sh_y)

p4 = gl.GLSurfacePlotItem(x=x[:,0], y = y[0,:], shader='heightColor', computeNormals=False, smooth=False)
p4.shader()['colorMap'] = np.array([0.2, 2, 0.5, 0.2, 1, 1, 0.2, 0, 2])
# сместить к краю
#p4.translate(10, 10, 0)
p4.scale(1,2,4)
w.addItem(p4)

# =======================
p4.setData(z=5*max_res/max_val)
# =======================

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()

