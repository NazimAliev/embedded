#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#
#

# drone_func.py

# import the necessary packages
import cv2
import os,sys
import numpy as np
import time
from scipy.spatial import distance
import drone_vars as v


# версия основанная на апроксимации контура эллипсом
def procContourEllipse(frame, cnt):
	el = cv2.fitEllipse(cnt)
	((x,y),(MA,ma),angle) = el
	prect = 2*(MA + ma)

	aspectRatio = MA / (ma + 0.01)

	# compute the solidity of the original cnt
	area = cv2.contourArea(cnt)
	hullArea = cv2.contourArea(cv2.convexHull(cnt))
	solidity = area / (float(hullArea)+0.01)

	# show moments
	if 0:
		M = cv2.moments(cnt)
		if M['m00'] != 0:
			cx,cy = int(M['m10']/M['m00']), int(M['m01']/M['m00'])
			#cv2.circle(frame,(cx,cy),3,255,-1)

	keepAspectRatio = aspectRatio >= v.ARMIN and aspectRatio <= v.ARMAX
	keepSolidity = solidity > v.SOLID
	#keepDims = w > v.W and h > v.H

	keepPerimeter = prect > v.PMIN and prect < v.PMAX
	if not(keepPerimeter):
		return None, None, None, None, None
	#if not(keepDims and keepSolidity and keepAspectRatio):
	#   continue
	if not(keepSolidity):
		return None, None, None, None, None

	#label = "     %d [%d/%.0f %.0f] %.0f" % (len(cnt), prect,aspectRatio,100.0*solidity,angle)
	#cv2.putText(frame, label, (int(x), int(y)), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0,0,200), 1)
	#cv2.ellipse(frame,el,(80,50,100),2)

	return x,y,MA,ma,angle

'''
# track[t] = np.roll(track[t],1,axis=0)
# track[t,0] = [fr,x,y,h,w,angle]
# track[:,0]: все последние fr,x,y,h,w,angle трека
track = np.zeros((TRACKS,DEPTH,6))

# состояние каждого трека:
# 0: не используется, свободен
# 1: активен
# [track_mode,track_len]
tmode = np.zeros((TRACKS,2))

пример добавления x = ((1,2),3)
tracks[0].append(x) = [[((1, 2), 3)]] первая точка нулевого трека
tracks[0].append(x) = [[((1, 2), 3), ((1, 2), 3)]] вторая точка нулевого трека
tracks[0].append(x) = [[((1, 2), 3), ((1, 2), 3), ((1, 2), 3)], []] еще раз точку в нулевой трек
a[1].append(x) = [[((1, 2), 3), ((1, 2), 3), ((1, 2), 3)], [((1, 2), 3)]] первую точку в новый первый трек
'''

# TODO: если для трека нет цели, она может появиться в следующий раз, тогда допуск на совпадение должен быть расширен
# TODO: более сложный алгоритм с предсказанием
# проверка оконтуренного и опрямоуголенного изображения rect
# на соответствие одному из сохраненных треков
def trackContours(rects, fr):

	# FIXME: test data
	if 0:
		rects = np.array([[30,40,0,0,0],[110,120,0,0,0],[510,520,0,0,0]])
		v.track[:,:,:] = 5000
		v.tmode[:] = 0
		v.track[3,0] = np.array([1,34,44,0,0,0])
		v.tmode[3] = [1,0]
		v.track[7,0] = np.array([3,1440,1540,0,0,0])
		v.tmode[7] = [1,0]
		v.track[8,0] = np.array([2,144,154,0,0,0])
		v.tmode[8] = [1,0]

	''' find nearest to given rect existing track '''
	rects_xy = np.array([rects[:,0],rects[:,1]]).T
	tracks_xy = np.array([v.track[:,0,1],v.track[:,0,2]]).T
	dist = distance.cdist(tracks_xy, rects_xy, 'euclidean')
	#v.log.debug('Dist: \n%s', dist)
	#idx = np.array(np.where(dist<5))
	# FIXME for runway
	idx = np.array(np.where(dist<5))
	#  idx = [[3 3 7 7] [0 1 0 1]]
	# track 3 match rect 0 and 1; track 7 match rect 0 and 1
	#v.log.debug('idx: %s dist shape: %s', idx, dist.shape)
	#v.log.debug('Index: \n%s\nDist[Index]:\n%s', idx, dist[idx])

	'''
	now, idx keep info about track <---> rect matching
	next:
	1. For matched tracks: update with rect
	2. For unmatched tracks < timeout: keep track with old rect
	3. For unmatched tracks > timeout: drop track
	4. For unmatched rects: start new track
	'''

	''' process matched tracks '''
	for d in idx.T:
		i_track = d[0]
		i_rect = d[1]
		# Сценарий 1: это обновление существующего трека
		#x,y,h,w,angle = rects[idx[:,1]]
		# shift track and free space [0] for next element
		v.track[i_track] = np.roll(v.track[i_track],1,axis=0)
		# write new element to track at the begin of array
		v.track[i_track,0] = np.append([fr],rects[i_rect])
		v.tmode[i_track,0] = 2
		v.tmode[i_track,1] += 1 
		#v.log.debug('[%d / %d] keep current track: %d', fr, i_track, v.tmode[i_track,0])

	''' process unmatched tracks '''	
	for t in range(v.TRACKS):
		
		# skip matched tracks
		matched = np.in1d(t, idx[0])
		if matched[0]:
			continue

		# tmode is track attribute, 0 means track is free
		if v.tmode[t,0] == 0:
			continue

		# plot broken - give chance to restore track 
		if v.tmode[t,0] == 2:
			v.track[t] = np.roll(v.track[t],1,axis=0)
			# write new element to track at the begin of array
			v.track[t,0] = v.track[t,1]
			v.tmode[t,0] = 3
			v.tmode[t,1] += 1 
			continue

		# plot was skiped before
		if v.tmode[t,0] >= 3:
			# inc timeout - how long we wait skipped interval
			v.tmode[t,0] += 1
			if v.tmode[t,0] >= v.TRACK_SKIPPED:
				# timeout, drop track
				v.tmode[t] = [0,0]
				v.track[t] = 0
			continue

		''' remove old track '''
		tfr,tx,ty,th,tw,tangle = v.track[t,0]
		if (fr - tfr) > v.TRACK_OBSOLETE:
			# трек устарел, подчищаем
			v.tmode[t] = [0,0]
			v.track[t] = 0
			#v.log.debug('[%d / %d] delete track: %d', fr, t, v.tmode[t,0])
			continue

	''' process unmatched rects '''	
	rt = -1
	for r in rects:
		rt += 1	
		# skip matched rects
		matched = np.in1d(rt, idx[1])
		if matched[0]:
			continue
		#v.log.debug('[%d / %d] unmatched rect: %s', fr, rt, r)
		# find empty track for this rect
		for t in range(v.TRACKS):
			if v.tmode[t,0] != 0:
				continue
			v.track[t,0] = np.append([fr],r)
			v.tmode[t] = [1,1] 
			# track len
			#v.log.debug('[%d / %d] new track created: [] -> %s', fr, t, v.tmode[t])
			break

	# catch wrong tracks for debug
	'''
	idx = np.where(v.track[:,1:3] != [0,0])[0]
	tr = v.track[idx]
	v.log.debug('\ntrack %s', tr[:,0,1:3])

	v.log.debug('\nprev track %s', tr[:,1,1:3])
	v.log.debug('\ncurrent track %s', tr[0,0,1:3])
	v.log.debug('\ncurrent tmode %s', v.tmode[idx])
	'''


# A: на сколько частей разбивается диапазон углов
# D: на сколько частей разбивается диапазон отрезков
def hough(A, D, max_tests):
	# accumulator
	AD = np.zeros((A, D))

	# max size in original terms
	rmax = np.hypot(v.frame_w, v.frame_h)
	dr = rmax / (D/2)   # delta for distances
	dth = np.pi / A     # delta for angles
	#print('rmax: %f, dr: %f, dth: %f' % (rmax, dr, dth))

	for p in v.points:
		[x, y] = p
		for alpha in range(A):
			th = dth * alpha
			r = x*np.cos(th) + y*np.sin(th)
			# dist changes from 0 to D
			dist = D/2 + int(r/dr+0.5)
			if alpha<A and dist<D:
				AD[alpha, dist] += 1
			else:
				v.log.debug('bad AD size [%d,%d]', alpha, dist)

	ads = np.empty((0,2))
	for i in range(max_tests):
		max_a, max_d = np.unravel_index(AD.argmax(), AD.shape)
		ads = np.append(ads, np.array([[max_a,max_d]]), axis=0)
		# exclude maximum point for the next iteration and
		# clear the same local maximums near the main maximum point
		delta = 2
		for j in range(-delta, delta+1):
			for k in range(-delta, delta+1):
				if max_a+j < A and max_d+k < D:
					AD[max_a+j,max_d+k]=0
	return ads

def findLines(frame, fr):
	#v.points = 10*np.array([[5,10,4,13,3,16,2,19,1,21,0,24],[0,28,2,26,4,24,6,22,8,20,10,18]]).T+200
	#v.points_l = np.zeros(12)

	lines = np.empty((0,2,2))
	rmax = np.hypot(v.frame_w, v.frame_h)
	A = 360
	D = 500
	# set of lines in r (perpendicular) and a (alpha angle) tuples
	ads = hough(A, D, max_tests = 5)
	if ads.size == 0:
		return None
	ko = rmax/D

	# convert from (r,a) coordinates to x1,y1,x2,y2 coordinates
	for p in ads:
		a = p[0]
		d = p[1]
		k = 2*ko*(d - D/2)
		fi = np.pi*a/A
		x0 = k*np.cos(fi)
		y0 = k*np.sin(fi)
		t = 600
		x1 = x0 + t * np.sin(fi) 
		y1 = y0 - t * np.cos(fi)
		x2 = x0 - t * np.sin(fi) 
		y2 = y0 + t * np.cos(fi)

		lines = np.append(lines, [[[x1,y1],[x2,y2]]], axis=0)
	return lines

'''
line: [ [[x1,y1],[x2,y2]],[],[] ]
'''

# points lie in lattice?
def deviation(points):
	if points.size <= 1:
		#v.log.debug('points is == 1')
		return 0
	sort = np.sort(points)
	diffs = np.diff(sort)
	dev = np.std(diffs)
	return dev

# межтрековая обработка
def trackAll(frame, fr):

	''' fill points[]: last points in every track '''
	z = np.zeros(0, dtype=int)
	v.points = np.empty((0,2))
	# point attribute: 0 means track is free
	v.points_t = np.empty(0)
	# point attribute: 0 means track not belong to any line, or number of line started from 1 
	v.points_l = np.empty(0)
	for t in range(v.TRACKS):
		if v.tmode[t,0] == 0:
			# no track, empty variable
			continue
		# слева - rect хвост всех треков
		tfr,tx,ty,th,tw,tangle = v.track[t,0]
		tlen = v.tmode[t,1]
		# TODO: игнорировать углы устаревающих треков
		if fr - tfr > 0:
			continue
		if tlen < 6:
			continue
		# add point
		v.points = np.append(v.points, np.array([[tx,ty]]), axis=0)
		# add track number to point
		v.points_t = np.append(v.points_t, t)
		# free place for point line number for using in future
		# if point will belong to line
		v.points_l = np.append(v.points_l, 0)

	if v.points.size == 0:
		return None

	''' detect raw lines from all points in the frame '''
	lines = findLines(frame, fr)
	v.houghlines = lines
	if lines == None:
		return

	''' assosiate points[]: track ends to raw lines and store line number in points_l[] '''
	cnt = 1
	for line in lines:

		# emulate line as set of many points for distance.cdist
		# based on x1,x2,y1,y2 input data
		x1 = line[0,0]
		y1 = line[0,1]
		x2 = line[1,0]
		y2 = line[1,1]
		#v.log.debug("[%d] (%d,%d) ---> (%d,%d)", cnt,x1,y1,x2,y2)
		x = np.linspace(x1, x2, 100)
		y = np.linspace(y1, y2, 100)
		linc = np.array([x,y])

		# dist between emulated line and track points
		dist = distance.cdist(linc.T, v.points, 'euclidean')

		# indexes of nearest to emulated line points
		idx = np.array(np.where(dist < 10))[1] 
		if idx.size == 0:
		#	v.log.debug("empty distance %s", line)
			return

		''' drop line if line not contain "lattice", periodical points '''
		x_points = v.points[idx,0]
		x_dev = deviation(x_points)
		y_points = v.points[idx,1]
		y_dev = deviation(y_points)
		#v.log.debug('dev (%f,%f)', x_dev, y_dev)
		DEVLEVEL = 60.0
		if x_dev < DEVLEVEL and y_dev < DEVLEVEL:
			# assign points belong to line with number cnt
			v.points_l[idx] = cnt
		else:
			# bad line - no regular points
			v.points_l[idx] = 0
		cnt += 1

# рисуем все 
def drawAll(frame):

	''' draw every track '''
	for t in range(v.TRACKS):
		if v.tmode[t,0] == 0:
			continue

		col = (10,10,10)
		size = 1
		if v.tmode[t,0] == 1:
			col = (100,0,0)
			size = 2
		if v.tmode[t,0] == 2:
			col = (0,100,0)
		if v.tmode[t,0] == 3:
			col = (0,255,255)
		tlen = int(v.tmode[t,1])
		#v.log.debug('tlen" %s', tlen)

		if tlen >= v.DEPTH:
			tlen = v.DEPTH

		for i in range(tlen-1):
			# если трек длинный, не рисуем начало (трек остывает)
			if i > tlen - 5:
				continue;
			# текущая точка
			tfc,x,y,h,w,angle = v.track[t][i]
			# предыдущая точка
			tfc0,x0,y0,h0,w0,angle0 = v.track[t][i+1]
			#(x,y) = ((x+x0)/2,(y+y0)/2)
			((x,y),(h,w)) = np.int0(((x,y),(h,w)))
			((x0,y0),(h0,w0)) = np.int0(((x0,y0),(h0,w0)))
			
			# for runway only - scale to right frame
			if 1:
				x = int(2.5*x - v.frame_w/2)
				y = int(2.5*y - v.frame_h/2)
				x0 = int(2.5*x0 - v.frame_w/2)
				y0 = int(2.5*y0 - v.frame_h/2)
			# отрисовка точки и линии в направлении движения
			cv2.circle(frame,(x,y),1,col,size)
			cv2.line(frame, (x0,y0), (x,y), col, 1)

	''' draw raw hough lines '''
	if 0 and v.houghlines != None:
		for line in v.houghlines: 
			[[x1,y1],[x2,y2]] = np.int0(line)

			cv2.line(frame,(x1,y1),(x2,y2),(50,255,100),1)

	''' draw latticed lines '''
	if 0:
		for cnt in range(1, len(v.points_l) + 1):
			# idx is inicies of points belong to line number cnt
			idx = np.where(v.points_l == cnt)[0]
			if idx.size == 0:
				continue
			mpoints = v.points[idx].astype(int)
			# edges of line: start and stop
			imin = np.argmin(mpoints[:,0]) 
			imax = np.argmax(mpoints[:,0]) 

			cv2.line(frame,(mpoints[imin,0],mpoints[imin,1]),(mpoints[imax,0],mpoints[imax,1]),(100,255,50),2)

	''' draw points '''
	if 0:
		i = 0
		# draw points - end of tracks. Plain points - black star.
		# If point belongs to line - draw different color
		for point in v.points.astype(int):
			#cv2.circle(frame,(mpoints[i,0],mpoints[i,1]),3,255,-1)
			if v.points_l[i] != 0:
				# point belongs to line: assign color
				cc = int(v.points_l[i] * 16)
				col = (cc,0,128)
			else:
				# plain point: black
				col = (0,0,0)	
			cv2.circle(frame,(point[0],point[1]),3,col,-1)
			label = "    [%d] %d x %d - %d" % (i, point[0], point[1], v.tmode[i,0])
			cv2.putText(frame, label, (point[0], point[1]), cv2.FONT_HERSHEY_SIMPLEX, 0.4, col, 1)
			i += 1

# версия основанная на апроксимации контура прямоугольником
def procContourRect(frame, cnt):
	# апроксимация контура ломаной 
	peri = cv2.arcLength(cnt, True)
	approx = cv2.approxPolyDP(cnt, 0.01 * peri, True)
	# определяем вращающийся прямоугольник, в который охватывает контур
	rect = cv2.minAreaRect(approx)
	((x,y),(h,w),angle) = rect
	# периметр объекта для фильтрации
	prect = 2*(w+h)

	angle = np.abs(angle)
	if w < h:
		(w, h) = (h, w)
		angle = 90 - angle
		# углы 89 и -89 рядом!
	aspectRatio = w / (float(h) + 0.01)

	# compute the solidity of the original cnt
	area = cv2.contourArea(cnt)
	hullArea = cv2.contourArea(cv2.convexHull(cnt))
	solidity = area / (float(hullArea)+0.01)
	if 0:
		M = cv2.moments(cnt)
		if M['m00'] != 0:
			cx,cy = int(M['m10']/M['m00']), int(M['m01']/M['m00'])
			#cv2.circle(frame,(cx,cy),3,255,-1)

	keepPerimeter = prect > v.PMIN and prect < v.PMAX
	keepAspectRatio = aspectRatio >= v.ARMIN and aspectRatio <= v.ARMAX
	keepSolidity = solidity > v.SOLID
	keepDims = w > v.W and h > v.H

	# количество кусков ломаной должно примерно соответствовать прямоугольнику 
	if len(approx) < v.RMIN or len(approx) > v.RMAX:
		return None, None, None, None, None
	if not(keepPerimeter):
		return None, None, None, None, None
	#if not(keepDims and keepSolidity and keepAspectRatio):
	#   continue
	if not(keepSolidity):
		return None, None, None, None, None

	#label = "     %d [%d/%.0f %.0f] %.0f" % (len(approx), prect,aspectRatio,100.0*solidity,angle)
	#cv2.putText(frame, label, (int(x), int(y)), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0,0,200), 1)
	if 1:
		# отрисовка контура
		box = cv2.boxPoints(rect)
		box = np.int0(box)
		cv2.drawContours(frame,[box],-1,(0,100,100),1)
		#cv2.polylines(frame, np.int32([rect]), 1, (255,255,255))

	return x,y,h,w,angle

