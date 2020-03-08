#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
#

# drone.py

# import the necessary packages
import cv2
import os,sys
import numpy as np
import time
import drone_vars as v
import drone_func as f
import drone_rw as r

'''
MAIN
'''

SAVEVIDEO = 0

loglevel = 0
LOG = v.log.CRITICAL
try:
	loglevel = int(os.environ['DRONELOGLEVEL'])
	print(loglevel)
except:
	print("DRONELOGLEVEL not set!")

if loglevel == 1:
	LOG = v.log.WARNING
if loglevel == 2:
	LOG = v.log.DEBUG
if loglevel == 3:
	LOG = v.log.INFO
	print("---")

v.log.basicConfig(stream=sys.stderr, format='%(levelname)s %(funcName)s:%(lineno)s: %(message)s', level=LOG)
v.log.debug("debug log level")

'''
# создаем фиксированное число треков
for t in range(v.TRACKS):
    v.tracks.append([])
    v.tmode.append(0)
'''

if len(sys.argv) == 2:
	video = sys.argv[1]
else:
	video = 0

# открываем видео-файл 
camera = cv2.VideoCapture(video)
v.frame_w = camera.get(cv2.CAP_PROP_FRAME_WIDTH);
v.frame_h = camera.get(cv2.CAP_PROP_FRAME_HEIGHT);
print("Video size: %dx%d" % (v.frame_w,v.frame_h))
 
if SAVEVIDEO:
	#out = cv2.VideoWriter('output2.avi',cv2.cv.CV_FOURCC(*'XVID'), 20.0, (int(v.frame_w),int(v.frame_h)))
	fourcc = cv2.VideoWriter_fourcc(*'MJPG')
	out = cv2.VideoWriter('output2.avi',fourcc, 30.0, (800,600))

# счетчик фреймов 
fr = 0
# для runway отдельный, потому что не совпадает
fr2 = 0
# признак самого первого трека
t1 = 0
# признак захвата ВПП
lock = False
while True:
	# берем фрейм из видео 
	(grabbed, frame0) = camera.read()
 
	# check to see if we have reached the end of the
	# video
	if not grabbed:
		break
	fr += 1
	''' 
	if v.DRONE == 1 and video != 0:
		frame[:, :300] = 0
		frame[-200:, :] = 0

	'''
	# не смотрим верхнюю часть кадра
	# там нет ВПП, отделяем границей
	# TODO граница зависит от высоты!
	if 0:
		frame0[:500, :] = 0
		frame0[:, -400:] = 0
	
	# window for runway
	if 0:
		frame0[:150, :] = 0
		frame0[550:, :] = 0
		frame0[:, :450] = 0
		frame0[:, 850:] = 0

	if lock:
		frame0[:400, :] = 0
		frame0[:, :400] = 0
		frame0[:, -400:] = 0

	# макс размер: размер окна нового фрейма
	# в который будет копироваться фрейм видео
	Xmax = 800
	Ymax = 600
	# сдвиг видео в новом фрейме относительно видео
	Xshift = 200
	Yshift = 200
	# поле отображения нового фрейма
	Xsize = 200
	Ysize = 200
	frame = np.zeros((Ymax,Xmax,3), dtype=np.uint8) 
	#frame[:500,:300] = frame0[:500,500:800]
	frame = frame0[Yshift:Ymax+Yshift,Xshift:Xmax+Xshift]

	#time.sleep(1)
	# фрейм в оттенки серого, размытие, нахождение границ 
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	# runway
	#blurred = cv2.GaussianBlur(gray, (9, 9), 0)
	# DJI
	#blurred = cv2.GaussianBlur(gray, (7, 7), 0)
	blurred = cv2.GaussianBlur(gray, (3, 3), 0)

	# runway img
	#edged = cv2.Canny(blurred, 10, 50)
	# DJI
	edged = cv2.Canny(blurred, 40, 130)
	(im2, cnts2, hi) = cv2.findContours(edged.copy(), cv2.RETR_LIST,
		cv2.CHAIN_APPROX_SIMPLE)

	lines = np.zeros((0,2,2))

	for c in cnts2:
		ls = r.findRunway(frame, c)
		if ls == None:
			continue
		lines = np.append(lines, ls,axis=0)
	if lines.size != 0:
		r.trackLines(lines, fr)
		fr2 += 1
		r.drawRW(frame)

	# контуры границ 
	(im2, cnts, hi) = cv2.findContours(edged.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)

	# show tracks only
	#frame[:,:] = 0 

	fc = 0
	rects = np.zeros((0,5))
	# цикл по контурам 
	for c in cnts:
		# лучше использовать эллипс, но он не строится когда ломаная меньше 5 точек
		# в этом случае работает прямоугольник.
		# структура rect в обоих случаях одна и та же
		'''	
		if len(c) < 5:
			x,y,w,h,angle = f.procContourRect(frame, c)
		else:
			x,y,w,h,angle = f.procContourEllipse(frame, c)
		'''	
		x,y,w,h,angle = f.procContourRect(frame, c)

		if x == None:
			continue
		rects = np.append(rects, np.array([[x,y,w,h,angle]]),axis=0)

	#f.trackContours(rects, fr)
 
	# тракинг всех контуров в взаимосвязи 
	'''
	# FIXME
	if fr % 2 == 0:
	'''
	f.trackAll(frame, fr)

	# рисуем треки и остальное
	f.drawAll(frame)

	cv2.imshow("Frame", frame)
	#cv2.imshow("Edged", edged)
	if SAVEVIDEO:
		out.write(frame)
	key = cv2.waitKey(1) & 0xFF
	if key == 27:
		break
	if key == 32:
		key = cv2.waitKey(5000) & 0xFF
	time.sleep(50.0 / 1000.0)	
 
# cleanup the camera and close any open windows
camera.release()
if SAVEVIDEO:
	out.release()
cv2.destroyAllWindows()
