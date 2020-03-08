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
from scipy.spatial import distance
import time

if len(sys.argv) == 2:
	video = sys.argv[1]
else:
	video = 0

# открываем видео-файл 
camera = cv2.VideoCapture(video)
# размер фрейма камеры из видеофайла
CW = camera.get(cv2.CAP_PROP_FRAME_WIDTH);
CH = camera.get(cv2.CAP_PROP_FRAME_HEIGHT);
# уменьшение размера нового фрейма
FW = CW - 1200
FH = CH - 400

''' Adaptation parameters '''
# количество каналов гистограммы
hist_nbins = 32 
# рассматриваем ограниченное количество цветов
# TODO: сделать ограничение по порогу интенсивности
DROP_LEVEL = 10 
# большие по площади объекты будут исключены начиная с этого уровня
# TODO: это должно зависеть от площади фрейма
HIST_HUE_LEVEL = 5000
# допуск по цвету для выделения маски канала 
MASK_HUE_LEVEL = 10
# мин и макс полупериметра bound rect.
# то что за пределами отбрасывается
PER_MIN = 20
PER_MAX = 200

''' Track parameters '''

TRACK_SKIPPED = 20
TRACK_OBSOLETE = 15
TRACK_ANGLE = 10
TRACK_HYPOT = 20
# максимальное число поддерживаемых треков
TRACKS = 30
# сколько последних точек каждого трека помним
# в частности надо для отрисовки трека. А для чего еще?
DEPTH = 10

# глобальная переменная для функции trackContour()
#tracks = [[]]

# замена для tracks. Хранит значения x,y
# сдвиг и добавление новой точки в сторону DEPTH:
# track[t] = np.roll(track[t],1,axis=0)
# track[t,0] = [fr,x,y,h,w]
# track[:,0]: все последние fr,x,y,h,w трека
track = np.zeros((TRACKS,DEPTH,5))

# состояние каждого трека:
# 0: не используется, свободен
# 1: активен
# [track_mode,track_len]
tmode = np.zeros((TRACKS,2))

# последние точки СУЩЕСТВУЮЩИХ треков, т.е. тех у которых tmode[t] != 0
# [[x,y]]
# x, y: point coordinates
points = np.empty((0,2))
# принадлежность каждой точки
# t:track number
points_t = np.empty(0)

# Параметры гистограммы
hist_height = 2*64
hist_width = 256
hist_bin_width = int(hist_width/hist_nbins)
# Маска если нужна
#hist_mask = np.zeros((FH,FW),  np.uint8)
hist_mask = None
# draw a circle in mask matrix
#cv2.circle(hist_mask,(int(FW/2),int(FH/2)), 50, 255, -1)

#Create array for the bins
hist_bins = np.arange(hist_nbins,dtype=np.int32).reshape(hist_nbins,1)

# фрейм для диаграммы гистограммы
hframe = np.zeros((hist_height,hist_width))

# получение и отрисовка гистограммы HSV цветов по одному признаку hue
def drawHist(frame):
	global hframe
	hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

	# получение гистограммы из фрейма и ее нормировка
	hist_hue = cv2.calcHist([hsv],[0],hist_mask,[hist_nbins],[0,180])
	hist_hue[np.where(hist_hue > HIST_HUE_LEVEL)[0]] = 0
	cv2.normalize(hist_hue,hist_hue,hist_height,cv2.NORM_MINMAX)
	hist=np.int32(np.around(hist_hue))
	pts = np.column_stack((hist_bins,hist))

	# отрисовка
	for x,y in enumerate(hist):
		cv2.rectangle(hframe,(x*hist_bin_width,y),(x*hist_bin_width + hist_bin_width-1,hist_height),(255),-1)

	#Flip upside down
	hframe = np.flipud(hframe)
	#Show the histogram
	cv2.imshow('Hist',hframe)
	hframe = np.zeros((hist_height,hist_width))
	return hist

# применение алгоритма CFAR к 1D гистограмме для поиска локальных максимумов
# результат:
# col_idxs: индексы локальных максимумов гистаграммы (цвета) найденные с помощью CFAR\
# z: значение гистограммы в локальном максимуме, сколько раз наблюдался этот цвет в кадре
def cfar(hist):
	h = hist[:,0]
	z = np.empty((h.size, 3))

	# подготовка соседних элементов для ячеек CUT с помощью матрицы
	# соседние относительно CUT ячейки будут в соседних массивах
	# это нужно для работы WHERE ко всему массиву
	z[:,0] = np.roll(h, -1)
	# CUT (Cell Under Test)
	z[:,1] = h
	z[:,2] = np.roll(h, +1)

	# индексы элементов которые больше элементов левого столбца
	wl = np.where(z[:,1] > z[:,0])
	# индексы элементов которые больше элементов правого столбца
	wr = np.where(z[:,1] > z[:,2])
	# индексы элементов которые больше элементов И левого И правого столбца
	col_idxs = np.intersect1d(wl[0], wr[0])
	cols = np.empty((col_idxs.size, 2))
	cols[:,0] = col_idxs
	cols[:,1] = z[col_idxs,1]
	return cols

# cols[:,0]: номера цветов (hue), точнее - индекс гистограммы
# cols[:,1]: интенсивность этих цветов
# отрисовка главных цветов фрейма из гистаграммы
def drawCols(cols):
	sz = cols.shape[0]
	if sz == 0:
		return
	# подготовка панели по количеству основных цветов 

	# упорядочение по интенсивности с максимума до минимума
	# для того чтобы на панели цветов слева направо 
	# начиналось с главных цветов
	ids = np.argsort(cols[:,1])[::-1]
	# цвета (индексы гистограммы)
	cds = cols[ids,0][:DROP_LEVEL]
	# их интенсивность
	zds = cols[ids,1][:DROP_LEVEL]
	# исключить определенный цвет
	wh = np.where(cds != 256)[0]
	# или оставить только определенный цвет
	#wh = np.where(cds == 10)[0]
	cds = cds[wh]
	zds = zds[wh]
	# фрейм для отображения цветов
	col_frame = np.empty((50,50*cds.size,3),dtype=np.uint8)
	# цвета из индексов
	hsvs = (180*cds)/hist_nbins

	font = cv2.FONT_HERSHEY_SIMPLEX
	# отрисовка hue из гистаграммы и добавление S или V
	for i in range(cds.size):
		hsv = [hsvs[i],255,255]
		col_frame[:,i*50:(i+1)*50] = hsv
	rgb = cv2.cvtColor(col_frame, cv2.COLOR_HSV2BGR)
	for i in range(cds.size):
		txt = "%d/%d" % (zds[i],cds[i])
		cv2.putText(rgb, txt,(6+i*50,30), font, 0.4,(0,0,0),1,cv2.LINE_AA)
	cv2.imshow('Color', rgb)
	colcut = np.empty((cds.size, 2))
	colcut[:,0] = cds
	colcut[:,1] = zds
	return colcut 


# функция вызывается для каждого канала, перечисление каналов в главном модуле
def findCnt(frame):
	''' Поиск контуров '''
	# фрейм в оттенки серого, размытие, нахождение границ 
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	blurred = cv2.GaussianBlur(gray, (3, 3), 0)
	#edged = cv2.Canny(blurred, 40, 130)
	edged = cv2.Canny(blurred, 20, 130)
	cv2.imshow("Canny", edged)
	# контуры границ 
	(im2, cnts, hi) = cv2.findContours(edged.copy(), cv2.RETR_LIST,
		cv2.CHAIN_APPROX_SIMPLE)

	rects = np.zeros((0,5),dtype=int)
	# цикл по контурам 
	for c in cnts:
		# апроксимация контура ломаной 
		peri = cv2.arcLength(c, True)
		if peri > 1000:
			continue
		approx = cv2.approxPolyDP(c, 0.01 * peri, True)
		if len(approx) > 10:
			continue
		#cv2.fillConvexPoly(frame, approx, (100,200,100)) 
		# определяем вращающийся прямоугольник, в который охватывает контур
		rect = cv2.minAreaRect(approx)

		#x,y,w,h = cv2.boundingRect(c)

		# отрисовка контура старым методом
		#box = cv2.boxPoints(rect)
		#box = np.int0(box)
		#cv2.drawContours(frame,[box],-1,(0,0,100),1)
		cv2.polylines(frame,[approx],True,(0,0,100))

		#cv2.rectangle(frame,(x,y),(x+w,y+h),(0,100,0))

		((x,y),(w,h),angle) = rect
		x = int(x)
		y = int(y)
		w = int(w)
		h = int(h)
		angle = int(angle)
		if w+h > PER_MIN and w+h < PER_MAX:
			rects = np.append(rects, np.array([[x,y,w,h,angle]]),axis=0)
	return rects 

def trackContours(rects, fr):

	# FIXME: test data
	if 0:

		track[:,:,:] = 5000
		tmode[:] = 0
		track[3,0] = np.array([1,34,44,0,0])
		tmode[3] = [1,0]
		track[7,0] = np.array([3,1440,1540,0,0])
		tmode[7] = [1,0]
		track[8,0] = np.array([2,144,154,0,0])
		tmode[8] = [1,0]

	''' find nearest to given rect existing track '''
	rects_xy = np.array([rects[:,0],rects[:,1]]).T
	tracks_xy = np.array([track[:,0,1],track[:,0,2]]).T
	dist = distance.cdist(tracks_xy, rects_xy, 'euclidean')
	# FIXME for runway
	idx = np.array(np.where(dist<5))
	#  idx = [[3 3 7 7] [0 1 0 1]]
	# track 3 match rect 0 and 1; track 7 match rect 0 and 1
	#log.debug('idx: %s dist shape: %s', idx, dist.shape)
	#log.debug('Index: \n%s\nDist[Index]:\n%s', idx, dist[idx])

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
		#x,y,h,w = rects[idx[:,1]]
		# shift track and free space [0] for next element
		track[i_track] = np.roll(track[i_track],1,axis=0)
		# write new element to track at the begin of array
		track[i_track,0] = np.append([fr],rects[i_rect])
		tmode[i_track,0] = 2
		tmode[i_track,1] += 1
		#log.debug('[%d / %d] keep current track: %d', fr, i_track, tmode[i_track,0])

	''' process unmatched tracks '''
	for t in range(TRACKS):

		# skip matched tracks
		matched = np.in1d(t, idx[0])
		if matched[0]:
			continue

		# tmode is track attribute, 0 means track is free
		if tmode[t,0] == 0:
			continue

		# plot broken - give chance to restore track 
		if tmode[t,0] == 2:
			track[t] = np.roll(track[t],1,axis=0)
			# write new element to track at the begin of array
			track[t,0] = track[t,1]
			tmode[t,0] = 3
			tmode[t,1] += 1
			continue

		# plot was skiped before
		if tmode[t,0] >= 3:
			# inc timeout - how long we wait skipped interval
			tmode[t,0] += 1
			if tmode[t,0] >= TRACK_SKIPPED:
				# timeout, drop track
				tmode[t] = [0,0]
				track[t] = 0
			continue

		''' remove old track '''
		tfr,tx,ty,th,tw = track[t,0]
		if (fr - tfr) > TRACK_OBSOLETE:
			# трек устарел, подчищаем
			tmode[t] = [0,0]
			track[t] = 0
			#log.debug('[%d / %d] delete track: %d', fr, t, tmode[t,0])
			continue

	''' process unmatched rects '''
	rt = -1
	for r in rects:
		rt += 1
		# skip matched rects
		matched = np.in1d(rt, idx[1])
		if matched[0]:
			continue
		#log.debug('[%d / %d] unmatched rect: %s', fr, rt, r)
		# find empty track for this rect
		for t in range(TRACKS):
			if tmode[t,0] != 0:
				continue
			track[t,0] = np.append([fr],r)
			tmode[t] = [1,1]
			# track len
			#log.debug('[%d / %d] new track created: [] -> %s', fr, t, tmode[t])
			break

# рисуем все 
def drawTracks(frame):

	''' draw every track '''
	for t in range(TRACKS):
		if tmode[t,0] == 0:
			continue

		col = (10,10,10)
		size = 1
		if tmode[t,0] == 1:
			col = (100,0,0)
			size = 2
		if tmode[t,0] == 2:
			col = (0,100,0)
		if tmode[t,0] == 3:
			col = (0,255,255)
		tlen = int(tmode[t,1])
		#log.debug('tlen" %s', tlen)

		if tlen >= DEPTH:
			tlen = DEPTH

		for i in range(tlen-1):
			# если трек длинный, не рисуем начало (трек остывает)
			if i > tlen - 5:
				continue;
			# текущая точка
			tfc,x,y,h,w = track[t][i]
			# предыдущая точка
			tfc0,x0,y0,h0,w0 = track[t][i+1]
			#(x,y) = ((x+x0)/2,(y+y0)/2)
			((x,y),(h,w)) = np.int0(((x,y),(h,w)))
			((x0,y0),(h0,w0)) = np.int0(((x0,y0),(h0,w0)))

			# for runway only - scale to right frame
			if 1:
				x = int(2.5*x - FW/2)
				y = int(2.5*y - FH/2)
				x0 = int(2.5*x0 - FW/2)
				y0 = int(2.5*y0 - FH/2)
			# отрисовка точки и линии в направлении движения
			cv2.circle(frame,(x,y),1,col,size)
			cv2.line(frame, (x0,y0), (x,y), col, 1)

