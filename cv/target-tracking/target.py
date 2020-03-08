#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
#

# import the necessary packages
import cv2
import os,sys
import numpy as np
import time
import target_func as f

'''
MAIN
'''

SAVEVIDEO = 0

# смещение нового фрейма относительно фрейма камеры
dx = 50
dy = 30

# фрейм камеры
cframe = np.zeros((f.CH,f.CW,3), dtype=np.uint8)
print("Video size: %dx%d" % (f.CW,f.CH))
# основной фрейм - crop фрейм камеры
frame = np.zeros((f.FH,f.FW,3), dtype=np.uint8)
print("Frame size: %dx%d" % (f.FW,f.FH))

# координаты основного фрейма
x1 = f.CW/2 + dx - f.FW/2
x2 = x1 + f.FW
y1 = f.CH/2 + dy - f.FH/2
y2 = y1 + f.FH

print("Frame x1,x2,y1,y2: %d %d %d %d" % (x1,x2,y1,y2))

if SAVEVIDEO:
	#out = cv2.VideoWriter('output2.avi',cv2.cv.CV_FOURCC(*'XVID'), 20.0, (int(v.frame_w),int(v.frame_h)))
	fourcc = cv2.VideoWriter_fourcc(*'MJPG')
	out = cv2.VideoWriter('output2.avi',fourcc, 30.0, (800,600))

# счетчик фреймов 
fr = 0
while True:
	# берем фрейм из видео 
	(grabbed, cframe) = f.camera.read()
	frame = cframe[y1:y2,x1:x2]
	#frame_rgb = np.zeros((f.FH,f.FW,3), dtype=np.uint8)
 
	# check to see if we have reached the end of the
	# video
	if not grabbed:
		break
	fr += 1
	rects = f.findCnt(frame)
	for i in range(rects[:,0].size):

		#f.trackContours(rects, fr)

		rect = rects[i,:]
		(x,y,w,h,angle) = rect
		#cv2.rectangle(frame,(x,y),(x+w,y+h),color=(100,100,100))

	'''
	hist = f.drawHist(frame)
	cols = f.cfar(hist)

	# фрейм с камеры в формате HSV, используется для выбора диапазона цветов
	hsv_orig = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
	channels = np.zeros(frame.shape, dtype=np.uint8)

	# преобладающие цвета
	colcut = f.drawCols(cols)
	# они же, в диапазоне HSV
	hsvs = (180*colcut[:,0])/f.hist_nbins
	cc = np.empty((1,1,3),dtype=np.uint8)

	# цикл по цветам
	for col in hsvs:
		if col > 256: 
			continue
		lower = np.array([col-f.MASK_HUE_LEVEL,50,50])
		upper = np.array([col+f.MASK_HUE_LEVEL,255,255])

		# маска участков фрейма, которые попали в диапазон преобладающих цветов 
		mask = cv2.inRange(hsv_orig, lower, upper)
		# получение канала исходного фрейма по маске 
		ch_hsv = cv2.bitwise_and(hsv_orig,hsv_orig, mask= mask)
		# поиск контура для канала
		ch_rgb = cv2.cvtColor(ch_hsv, cv2.COLOR_HSV2BGR)

		# получили массив прямоугольников для каждого цвета
		rects = f.findCnt(ch_rgb)

		# отрисовка массива прямоугольников цвета col
		cc[0,0,:] = [col,100,100]
		rc = cv2.cvtColor(cc, cv2.COLOR_HSV2BGR)
		rc2 = np.int0(rc[0,0])
		rc3 = tuple(map(int, rc2))
		for i in range(rects[:,0].size):

			#f.trackContours(rects, fr)

			rect = rects[i,:]
			(x,y,w,h) = rect
			cv2.rectangle(ch_rgb,(x,y),(x+w,y+h),color=rc3)

		# добавление канала в фрейм каналов
		frame_rgb = cv2.bitwise_or(frame_rgb,ch_rgb)

	#f.drawTracks(frame)
	'''

	cv2.imshow("Frame", frame)
	#cv2.imshow("Mask", frame_rgb)

	if SAVEVIDEO:
		out.write(frame)
	key = cv2.waitKey(1) & 0xFF
	if key == 27:
		break
 
# cleanup the camera and close any open windows
f.camera.release()
if SAVEVIDEO:
	out.release()
cv2.destroyAllWindows()

