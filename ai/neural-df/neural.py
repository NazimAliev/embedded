#!/usr/bin/python
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import os,sys
import numpy as np
import nemi as nemi
import network as nw
import matplotlib.pyplot as plt

np.set_printoptions(precision=2)
KNOISE = 0.05*2*np.pi*nemi.RL

'''
MAIN
'''
# ROWS: количество азимутов по которым собираются данные
# TRAINS*ROWS: столько всего выборок с решетки
tmp = np.zeros(nemi.DIPOLS)
#tmpd = np.zeros(2*nemi.DIPOLS)
train = []
desired = []
# подготовка нулевого желаемого вектора, единица устанавливается в цикле
# считывание всех тренировочных данных в цикле
# это имеет смысл, потому что каждый раз меняется шум
# повторяется выборка
BASE = 8
TRAINS = 100
for t in range(TRAINS):
	for q in range(nemi.ROWS):
		#q = np.random.randint(nemi.ROWS, size=nemi.DIPOLS)
		rowzero = np.zeros(nemi.ROWS)
		# все массивы длиной DIPOLS
		dipol, phase = nemi.getDipols(q)
		phase += np.random.normal(0,KNOISE,nemi.DIPOLS) 
		plist = nemi.phase2level(phase, BASE)
		# подгонка под формат входных данных пакета network
		a = [[i] for i in plist]
		train = train + [a]
		rowzero[q] = 1
		b = [[i] for i in rowzero.tolist()]
		desired = desired + [b] 

train = np.array(train)
desired = np.array(desired)

td = zip(train, desired)
#print(np.array(td[0][0]).reshape(nemi.DIPOLS,BASE))
#print(td[30][1])

# *********************************************
#net = nw.Network([BASE*nemi.DIPOLS, nemi.ROWS])
net = nw.Network([BASE*nemi.DIPOLS, 2*nemi.ROWS, nemi.ROWS])
net.SGD(td, 30, nemi.ROWS, 5.0)
phi = np.zeros((BASE*nemi.DIPOLS,1))
Q = 9
print("DIPOLS=%d ROWS=%d Q=%d points, %.0f deg" % (nemi.DIPOLS,nemi.ROWS,Q,360*Q/nemi.ROWS))
d, f = nemi.getDipols(Q)
plist = nemi.phase2level(f, BASE)
phi = np.array([[i] for i in plist])
res = net.feedforward(phi)
print(res.shape)
w = res[:,0]
# *********************************************
if 1:
    plt.figure(1)
    XFILE = 2*np.pi*np.arange(nemi.ROWS)/nemi.ROWS
    plt.polar(XFILE, w, 'bo-')
if 1:
    plt.figure(2)
    #XFILE = 360.0*np.arange(nemi.ROWS)/nemi.ROWS
    #plt.plot(XFILE, w, 'bo-')
    plt.plot(w, 'bo-')

imax = np.argmax(w)
print("DOA: %.2f" % (360.0*imax/nemi.ROWS))

'''
plt.figure(2)
plt.plot(np.absolute(eg)/100, 'r', label="Error abs")
leg = plt.legend()
leg.get_frame().set_alpha(0.5)
'''

plt.grid(True)
plt.show()

