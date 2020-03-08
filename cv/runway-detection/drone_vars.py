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
import logging as log
import time

# to see all warnings as errors and raise exception
#np.seterr(all='raise')

# TODO масштаб прямоугольников должен меняться в зависимости от приближения к полосе
# TODO настройки трекера должны меняться в зависимости от скорости, т.е. от расстояния до полосы

DRONE = 1
VENEZIA = 0
# граничные условия для выбора подходящих прямоугольников на ВПП

if DRONE:
	# мин. и макс. количество кусков ломаной 
	RMIN = 4
	RMAX = 12 
	# мин. и макс. значение периметра прямоугольника
	PMIN = 2 
	PMAX = 200
	W = 1 # минимальная ширина
	H = 1 # минимальная высота
	# мин. и макс. отношения высоты к ширине
	# у прямоугольника ширина всегда больше высоты
	ARMIN = 1 
	ARMAX = 100
	SOLID = 0.005	# мин. толщина линии прямоугольника
	TRACK_SKIPPED = 20 
	TRACK_OBSOLETE = 15
	TRACK_ANGLE = 10
	TRACK_HYPOT = 20

# максимальное число поддерживаемых треков
TRACKS = 300
# сколько последних точек каждого трека помним
# в частности надо для отрисовки трека. А для чего еще?
DEPTH = 10

# глобальная переменная для функции trackContour()
#tracks = [[]]

# замена для tracks. Хранит значения x,y
# сдвиг и добавление новой точки в сторону DEPTH:
# track[t] = np.roll(track[t],1,axis=0)
# track[t,0] = [fr,x,y,h,w,angle]
# track[:,0]: все последние fr,x,y,h,w,angle трека
track = np.zeros((TRACKS,DEPTH,6))

# состояние каждого трека:
# 0: не используется, свободен
# 1: активен
# [track_mode,track_len]
tmode = np.zeros((TRACKS,2))
frame_w = 0
frame_h = 0
# последние точки СУЩЕСТВУЮЩИХ треков, т.е. тех у которых tmode[t] != 0
# [[x,y]]
# x, y: point coordinates
points = np.empty((0,2))
# принадлежность каждой точки
# t:track number
points_t = np.empty(0)
# l: line assosiate
points_l = np.empty(0)
houghlines = np.empty((0,2,2))

# for runway
rwtrack = np.zeros((TRACKS,DEPTH,6))
rwtmode = np.zeros((TRACKS,2))
rwlines = np.empty((0,0,0,0))
# as tmode but without empty
rwmode = np.empty((0,2))
