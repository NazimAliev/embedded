#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
#

# gradient.py

'''
find gradient in blurred circle image
'''
 
# import the necessary packages
import cv2

import numpy as np
from matplotlib import pyplot as plt

# if picture not blurred gradient is wrong
BLURRED = True 

# strong bw image
pic = cv2.imread('/data/spiral.png',0)

if BLURRED:
	kernel = np.ones((5,5),np.float32)/25
	#blurred = pic 
	blurred = cv2.filter2D(pic,-1,kernel)
else:
	blurred = pic 

# gx,gy are arrays of x,y for each gradient vector
gx = cv2.Sobel(blurred,cv2.CV_64F,1,0,ksize=3)/20
gy = cv2.Sobel(blurred,cv2.CV_64F,0,1,ksize=3)/20

angle = cv2.phase(gx, gy, angleInDegrees=False)

xy = pic.shape
print(xy)
X,Y = xy 

# temporary field for draw line
tmp = np.zeros(xy, dtype=np.uint8)
# accumulator to keep intersected lines points
acc = np.zeros(xy, dtype=np.uint16)

i = 0
#scan for all gradients
for x in range(0,X,4):
	for y in range(0,Y,4):
		# skip zero pixels
		if(np.hypot(gx[x,y],gy[x,y])) > 10:
			#print('[%d,%d] -> %.0f' % (x,y, 180*angle[x,y]/np.pi))
			#pt1 = (y,x)
			# draw gradient line
			pt1 = (y,x)
			pt2 = (y+int(gx[x,y]),x+int(gy[x,y]))
			# only for show
			#cv2.line(pic, pt1,pt2, (100,100,100), 1)
			cv2.arrowedLine(pic, pt1,pt2, (100,100,100), 1)
			# for accumulator
			cv2.line(tmp, pt1,pt2, (1,1,1), 2)
			# add gradient line points to accumulator
			acc += tmp
			tmp[:,:] = 0
			i += 1	

print('%d cycles' % i)
# max of accumulator is concentric circles center
acc_max = np.amax(acc)
acc_argmax = np.argmax(acc)
argmax2d = np.unravel_index(acc_argmax, xy)
print('%d accumulator max' % acc_max)
print('[%d,%d] accumulator point' % argmax2d)
# put marker on image
cv2.circle(acc,(argmax2d[1],argmax2d[0]),15,(10,10,10),2)

plt.figure(0)
plt.imshow(pic,cmap = 'gray')
plt.show()
