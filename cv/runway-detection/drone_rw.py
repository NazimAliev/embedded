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
from scipy.spatial import distance
import drone_vars as v

# TODO: если для трека нет цели, она может появиться в следующий раз, тогда допуск на совпадение должен быть расширен
# TODO: более сложный алгоритм с предсказанием

# find nearest to given rect existing rwtrack
def matchRect2Track(rects):
	rects_xy = np.array([rects[:,0],rects[:,1]]).T
	rwtracks_xy = np.array([v.rwtrack[:,0,1],v.rwtrack[:,0,2]]).T
	dist = distance.cdist(rwtracks_xy, rects_xy, 'euclidean')
	v.log.debug('\nrects_xy:\n%s',rects_xy)
	v.log.debug('\nrwtracks_xy:\n%s',np.where(rwtracks_xy[:,0] != 0)[0])
	#v.log.debug('Dist: \n%s', dist)
	#idx = np.array(np.where(dist<5))
	# FIXME for runway
	idx = np.array(np.where(dist<5))
	#  idx = [[3 3 7 7] [0 1 0 1]]
	# rwtrack 3 match rect 0 and 1; rwtrack 7 match rect 0 and 1
	#v.log.debug('idx: %s dist shape: %s', idx, dist.shape)
	#v.log.debug('Index: \n%s\nDist[Index]:\n%s', idx, dist[idx])
	return idx

# find closely rects 
def matchRect2Rect(rects):
	rects_xy = np.array([rects[:,0],rects[:,1]]).T
	dist = distance.cdist(rects_xy, rects_xy, 'euclidean')
	v.log.debug('\nrects_xy:\n%s',rects_xy)
	idx = np.array(np.where(dist<5))
	return idx

# add rect to track[itrack]
def addRectToTrack(fr, rect, itrack, tmode):
	if rect.shape != 5:
		v.log.error('\n[%d] rect size error', fr)
		return
	# shift rwtrack and free space [0] for next element
	v.rwtrack[itrack] = np.roll(v.rwtrack[itrack],1,axis=0)
	# write new element to rwtrack at the begin of array
	v.rwtrack[itrack,0] = np.append([fr],rect)
	v.rwtmode[itrack,0] = 2
	v.rwtmode[itrack,1] += 1 
	v.log.debug('\n[%d] rect %s to track %d, tmode=%d', fr, rect, itrack, tmode)

def dropTrack(fr, itrack):
	v.rwtmode[itrack] = [0,0]
	v.rwtrack[itrack] = [fr, 0, 0, 0, 0] 

def trackLines(lines, fr):
	#if fr > 7:
	#	exit(0)
	#v.log.debug('\nrf[%d] ===== input lines \n%s', fr, lines)

	rects = np.zeros((lines.shape[0], 5))
	# x0,y0
	rects[:,0:2] = lines[:,0,:]
	# w,h
	rects[:,2:4] = lines[:,1,:] - lines[:,0,:]
	# angle
	rects[:,4] = np.angle(rects[:,2] + 1j*rects[:,3], deg=True) 

	''' match rect to tracks '''
	idx = matchRect2Track(rects)
	v.log.debug('\nidx:\n%s',idx)

	'''
	now, idx keep info about rwtrack <---> rect matching
	next:
	1. For matched rwtracks: update with rect
	2. For unmatched rwtracks < timeout: keep rwtrack with old rect
	3. For unmatched rwtracks > timeout: drop rwtrack
	4. For unmatched rects: start new rwtrack
	'''

	''' process matched rwtracks '''
	for d in idx.T:
		i_rwtrack = d[0]
		i_rect = d[1]
		# Сценарий 1: это обновление существующего трека
		#x,y,h,w,angle = rects[idx[:,1]]

		''' add rect to track[itrack] '''
		addRectToTrack(fr, rects[i_rect], i_rwtrack, 2)

	''' process unmatched rwtracks '''	
	for t in range(v.TRACKS):
		
		# skip matched rwtracks
		matched = np.in1d(t, idx[0])
		if matched[0]:
			v.log.debug('\n[%d], matched tracks: %s\n%s', t, matched,v.rwtrack[t,0])
			continue

		# rwtmode is rwtrack attribute, 0 means rwtrack is free
		if v.rwtmode[t,0] == 0:
			continue

		# plot broken - give chance to restore rwtrack 
		if v.rwtmode[t,0] == 2:

			''' add prev rect to track[itrack] '''
			addRectToTrack(fr, v.rwtrack[t,1], t, 3)

			continue

		# plot was skiped before
		if v.rwtmode[t,0] >= 3:
			# inc timeout - how long we wait skipped interval
			v.rwtmode[t,0] += 1
			if v.rwtmode[t,0] >= v.TRACK_SKIPPED:
	
				''' timeout for skipped plot, drop track '''
				dropTrack(fr, t)
			continue

		''' remove old rwtrack '''
		tfr,tx,ty,th,tw,tangle = v.rwtrack[t,0]
		if (fr - tfr) > v.TRACK_OBSOLETE:

			''' timeout for skipped plot, drop track '''
			dropTrack(fr, t)

			continue

	''' process unmatched rects '''	

	# we should drop close rects
	idx = matchRect2Rect(rects)
	v.log.debug('\nDIST rects closely:\nidx: %s\nrects[idx]: %s',idx,rects[idx,0:2])

	rt = -1
	for r in rects:
		rt += 1	
		# skip matched rects
		matched = np.in1d(rt, idx[1])
		if matched[0]:
			continue
		#v.log.debug('[%d / %d] unmatched rect: %s', fr, rt, r)
		# find empty rwtrack for this rect
		for t in range(v.TRACKS):
			if v.rwtmode[t,0] != 0:
				continue

			''' create new track '''
			addRectToTrack(fr, r, t, 1)
			break

	# catch wrong rwtracks for debug
	'''
	idx = np.where(v.rwtrack[:,1:3] != [0,0])[0]
	tr = v.rwtrack[idx]
	v.log.debug('\nrwtrack %s', tr[:,0,1:3])

	v.log.debug('\nprev rwtrack %s', tr[:,1,1:3])
	v.log.debug('\ncurrent rwtrack %s', tr[0,0,1:3])
	v.log.debug('\ncurrent rwtmode %s', v.rwtmode[idx])
	'''
	idx = np.where(v.rwtrack[:,0,1:3] != [0,0])[0]
	tr = v.rwtrack[idx]
	# v.rwlines[NOT_EMPTY_TRACK_INDEX,N_POINT_IN_TRACK,[[x,y],[w,h]]]
	v.rwlines = np.zeros((idx.size, v.DEPTH, 2, 2))
	# x0,y0
	v.rwlines[:,:,0,0:2] = tr[:,:,1:3]
	# w,h
	v.rwlines[:,:,1,0:2] = tr[:,:,1:3] + tr[:,:,3:5]
	v.rwmode = v.rwtmode[idx,:]
	v.log.debug('\noutput rwlines:\n%s\n%s',v.rwlines[:,0,0,0:2],v.rwmode)

# filter for edge line - should be vertical - simular
def ahRules(line):
	[[x0,y0],[x,y]] = line
	th = np.angle((x - x0) + 1j*(y - y0), deg=True)
	# filter lines by angles - we need runway edges, 4 lines
	rule1 = th > 55 and th < 180 - 55
	rule2 = th < -55 and th > -180 + 55
	rule3 = th < 10 and th > 180 - 10 
	rule4 = th > -10 and th < -180 + 10 
	rule5 = th > 80 and th < 180 - 80 
	rule6 = th < -80 and th > -180 + 80 
	if not (rule1 or rule2 or rule5 or rule6):
		return False
		# filter lines by lenghts
	hyp = np.hypot((x - x0), (y - y0))
	if hyp < 20 or hyp > 80:
		return False 

	return True 

# convert approx data to lines, filter lines by runway edge criteria
# approx example (3 lines):
# [[[940, 522], [960, 462]], [[1017, 452], [1042, 522]], [[1010, 452], [1032, 517]]]
def rwFilter(approx):
	# sort lines
	if len(approx) == 0:
		return 0, None, None
	i = -1 # for skipping first point
	# prev point
	x0 = 0
	y0 = 0
	# points in cnt
	lock = 0
	lines = []
	# approx consist of xy points, not lines
	for a in approx:
		i += 1
		x = a[0][0]
		y = a[0][1]
		# x,y should be UP to calculate horizontal position
		if y0 < y:
			line = [[x0,y0],[x,y]]
		else:
			line = [[x,y],[x0,y0]]
		# assign prev values
		if i != 0:
			# edge line? drop others
			res = ahRules(line)
			if res:
				lines.append(line)
				lock += 1
		x0 = x
		y0 = y

	return lock, lines

def scale(xy):
	#out = np.int0(2.5*np.array(xy) - [v.frame_w/2, v.frame_h/2]) 
	out = np.int0(2.5*np.array(xy) - [-100, 500]) 
	return out

# TODO: remove drawing to drawAll
def findRunway(frame, c):
	# perimeter of cnt
	peri = cv2.arcLength(c, False)
	# filter by perimeter
	if peri < 30 or peri > 160:
		return None	
	# approximation of cnt
	#eps = 0.005 * peri
	eps = 300 * 0.05
	approx = cv2.approxPolyDP(c, eps, closed=False)
	# all contours
	cc = c
	cv2.drawContours(frame,[cc],0,(0,50,255),1)

	''' ==== rwFilter ==== '''
	lock,lines = rwFilter(approx)

	if lock == 0:
		return None
	v.log.debug('\ncnt: %s',c)
	v.log.debug('\napprox: %s',approx)

	# drawing
	# zoom approx to detailed see in screen
	approx_scaled = scale(approx)

	if len(approx) != 0:
		# filtered contours in main picture
		cv2.drawContours(frame,[approx],0,(255,50,0),1)
		# scaled contours in right frame
		#cv2.drawContours(frame,[approx_scaled],0,(255,50,0),1)

		x,y,w,h = cv2.boundingRect(c)
		label = '    %d' % (peri)
		cv2.putText(frame, label, (int(x), int(y)), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (200,0,0), 1)
		#cv2.rectangle(frame,(x,y),(x+w,y+h),(0,255,0),2)

	return lines

def drawRW(frame):

	depth = 4
	rwtracks = np.array((depth,8))
	xy = np.empty((0,2)) 
	xmin = 5000
	xmax = 0
	ymin = 5000
	ymax = 0
	i = -1
	igood = -1
	for l in v.rwlines:
		i += 1
		#v.log.debug('%d',i)

		col = (10,10,10)
		size = 2
		if v.rwmode[i,0] == 1:
			col = (100,0,0)
		if v.rwmode[i,0] == 2:
			col = (0,100,0)
		if v.rwmode[i,0] == 3:
			col = (0,255,255)
		tlen = int(v.rwmode[i,1])

		if tlen >= v.DEPTH:
			tlen = v.DEPTH

		for j in range(tlen-1):
			# если трек длинный, не рисуем начало (трек остывает)
			if j > tlen - 5:
				continue;
			# текущая точка
			[[x0,y0],[x,y]] = scale(l[j])
			# предыдущая точка
			[[x10,y10],[x1,y1]] = scale(l[j+1])
			
			# отрисовка точки и линии в направлении движения
			cv2.circle(frame,(x,y),1,col,size)
			cv2.line(frame, (x1,y1), (x,y), col, 1)

		# short track may be random: don't draw
		if v.rwmode[i,1] < 15:
			continue
		igood += 1

		x0 = np.mean(l[:,0,0])	
		y0 = np.mean(l[:,0,1])	
		x = np.mean(l[:,1,0])	
		y = np.mean(l[:,1,1])	
		[x0,y0] = scale([x0,y0])
		[x,y] = scale([x,y])

		if x < xmin: xmin = x
		if x > xmax: xmax = x
		if y0 < ymin: ymin = y0
		if y > ymax: ymax = y

		cv2.line(frame,(int(x),int(y)),(int(x0),int(y0)),(0,255,0),1)

	d = 10
	cv2.rectangle(frame,(320,250),(600,500),(0,255,0),1)
	cv2.rectangle(frame,(xmin-d,ymax+d),(xmax+d,ymin-d),(100,255,0),1)
	label = '    %d' % (igood)
	cv2.putText(frame, label, (xmin, ymin), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (200,0,0), 1)

	return
