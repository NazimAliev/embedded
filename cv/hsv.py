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

if len(sys.argv) == 2:
	video = sys.argv[1]
else:
	video = 0

'''
green = np.uint8([[[0,255,0 ]]])
hsv_green = cv2.cvtColor(green,cv2.COLOR_BGR2HSV)
print hsv_green
'''

video = "/data/pilot.mp4"
# открываем видео-файл 
camera = cv2.VideoCapture(video)

#HI = [130,255,255]
#LO = [110,50,50]

LO = [0,0,0]
HI = [255,60,255]

while(1):

	# Take each frame
	_, frame = camera.read()

	hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
	height, width = frame.shape[:2]
	color = hsv[int(width/2)][int(height/2)]
	#print(color)

	# define range of blue color in HSV
	lo= np.array(LO, dtype=np.uint8)
	up= np.array(HI, dtype=np.uint8)

	# Threshold the HSV image to get only blue colors
	mask = cv2.inRange(hsv, lo, up)
	#mask = 255 - mask

	# Bitwise-AND mask and original image
	res = cv2.bitwise_and(frame,frame, mask= mask)

	#cv2.imshow('frame',frame)
	#cv2.imshow('mask',mask)
	cv2.imshow('res',res)
	k = cv2.waitKey(5) & 0xFF
	if k == 27:
		break

cv2.destroyAllWindows()
