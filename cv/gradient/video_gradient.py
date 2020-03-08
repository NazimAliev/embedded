#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
#

# video_gradient.py

'''
find gradient in blurred circle video 
'''
 
# import the necessary packages
import cv2
import numpy as np

video = 0
camera = cv2.VideoCapture(video)
frame_w = int(camera.get(cv2.CAP_PROP_FRAME_WIDTH));
frame_h = int(camera.get(cv2.CAP_PROP_FRAME_HEIGHT));
print("Video size: %dx%d" % (frame_w,frame_h))

xy = (frame_w,frame_h)
# temporary field for draw line
tmp = np.zeros(xy, dtype=np.uint8)
# accumulator to keep intersected lines points
acc = np.zeros(xy, dtype=np.uint16)

# acc size = img size in normal mode
# img decreased in locked mode, acc size keep the same
def findGradient(img):
	global acc, tmp
	h,w = img.shape
	(kh,kw) = h//5,w//5
	# gx,gy are arrays of x,y for each gradient vector
	gx = cv2.Sobel(img,cv2.CV_64F,1,0,ksize=5)/kh
	gy = cv2.Sobel(img,cv2.CV_64F,0,1,ksize=5)/kw

	acc[:,:] = 0
	for x in range(0,h,8):
		for y in range(0,w,8):
			if(np.hypot(gx[x,y],gy[x,y])) > 10:
				#print('[%d,%d] -> %.0f' % (x,y, 180*angle[x,y]/np.pi))
				#pt1 = (y,x)
				pt1 = (y-int(gx[x,y]),x-int(gy[x,y]))
				pt2 = (y+int(gx[x,y]),x+int(gy[x,y]))
				#print(180*angle[x,y]/np.pi)
				#pt2 = (x+int(sy[x,y]),y+int(sx[x,y]))
				#cv2.arrowedLine(img, pt1,pt2, (100,100,100), 1)
				cv2.line(img, pt1,pt2, (100,100,100), 1)
				cv2.line(tmp, pt1,pt2, (10,10,10), 1)
				acc += tmp
				tmp[:,:] = 0

	acc_max = np.amax(acc)
	acc_argmax = np.argmax(acc)
	argmax2d = np.unravel_index(acc_argmax, xy)
	return acc_max, argmax2d

locked = True 
xlock,ylock = frame_w//2,frame_h//2
wlock,hlock = (100,200) 
lock_window = np.zeros((wlock,hlock), dtype=np.uint8)
H = frame_h
W = frame_w

prev_img = np.zeros((frame_h,frame_w), dtype=np.uint8) 
sub_img = np.zeros((frame_h,frame_w), dtype=np.uint8) 
s1 = np.zeros((frame_h,frame_w), dtype=np.uint8) 
#s2 = np.zeros((frame_h,frame_w), dtype=np.uint8) 
#s3 = np.zeros((frame_h,frame_w), dtype=np.uint8) 

fr = 0
flip = False 
while(1):
	grabbed, frame_bgr = camera.read()
	if not grabbed:
		# when timer is more fast than frame rate
		continue	
	frame_gray = cv2.cvtColor(frame_bgr,cv2.COLOR_BGR2GRAY)
	blurred = cv2.GaussianBlur(frame_gray, (5, 5), 0)
	if (fr % 3 == 0):
		if(flip):
			flip = False
			col = (200,200,200)
		else:
			flip = True
			col = (20,20,20)

	cv2.circle(blurred, (100,100),20, col,-1, 1)
	ret,img = cv2.threshold(blurred,127,255,cv2.THRESH_BINARY)
	#img = cv2.Canny(th, 20, 60)

	sub_img = img - prev_img
	tmp = (sub_img + s1 + s2 + s3)//4
	s3 = s2
	s2 = s1
	s1 = sub_img
	prev_img = img
	cv2.putText(tmp, '%0d' % fr, (20,20), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255,255,255), 1)
	cv2.imshow('Img', img)
	cv2.imshow('Sub', tmp)


	'''
	if locked:
		lock_window = img[xlock - wlock/2 : xlock + wlock/2, ylock - hlock/2 : ylock + hlock/2]
		acc_max, argmax2d = findGradient(lock_window)
		print('LOCKED %d accumulator max' % acc_max)
	else:
		acc_max, argmax2d = findGradient(img)
		print('no lock %d accumulator max' % acc_max)
	if(acc_max > 200):
		xlock,ylock = argmax2d
		if(xlock < wlock//2):
			xlock = wlock//2
		if(ylock < hlock//2):
			ylock = hlock//2
		locked = True
	else:
		xlock,ylock = frame_w//2,frame_h//2
		locked = False 

	#print('[%d,%d] accumulator point' % argmax2d)
	cv2.circle(img,(argmax2d[1],argmax2d[0]),15,(10,10,10),2)

	cv2.imshow('Lock', lock_window)
	cv2.imshow('Img', img)
	'''

	key = cv2.waitKey(1) & 0xFF
	if key == 27:
	   break 
	fr += 1

# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()
exit(0)
