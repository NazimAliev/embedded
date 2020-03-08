#!/usr/bin/python
# -*- coding: utf8 -*-

# Пакет с общими функциями
import numpy as np

DIPOLS = 8 
ROWS = 16 
RL = 0.15        # R/lambda
# параметр адаптивного фильтра, определяет сходимость
STEP = 0.005

''' массив из DIPOLS элементов '''
def getDipols(Q):
	i = np.arange(DIPOLS)
	phase = 2*np.pi*RL*np.cos(2*np.pi*i/DIPOLS - 2*np.pi*Q/ROWS)
	sig = 1.0 * np.exp(1j*phase)
	return(sig, phase)

''' Тренировка решетки пилотом '''
def train(pilot, d, w):
	# выход фильтра
	y = np.sum(pilot*np.conj(w))
	e = d - y

	# пересчет W
	w += STEP*pilot*np.conj(e)
	return w, y, e

''' Преобразование фаз в уровни '''
def phase2level(phase, base):
	p = phase
	# максимум среди отрицательных фаз, чтобы приподнять все фазы > 0
	m = np.abs(p[p < 0]).max()
	# приподняли фазы чтобы не было отрицательных значений
	p += m
	# масштабировать фазы чтобы максимум был base
	p *= (base - 1) / p.max()
	# делаем из p индексы зависящие от величины фазы
	k = np.rint(p).astype(int)

	l = []
	# для каждой фазы создается массив с одной единицей
	for i in range(DIPOLS):	
		pl = [0]*base
		pl[k[i]] = 1
		l = l + pl
	return l
