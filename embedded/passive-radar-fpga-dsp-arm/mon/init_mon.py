#!/usr/bin/python3

# -*- coding: utf-8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


"""
Инициализация для основного приложения main_mon.py
"""

from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
from multiprocessing import Pool
import socket as sc
import numpy as np
import func_mon as fm
import sys

if len(sys.argv) != 4:
	sys.exit('Usage: %s frames {zboard | dsp | both} tcp_port' % sys.argv[0])

FRAMES = int(sys.argv[1])
MODE = sys.argv[2]
PORT = int(sys.argv[3])

if (MODE != "zboard" and MODE != "dsp" and MODE != "both"):
	sys.exit('Usage: %s frames {zboard | dsp | both} tcp_port' % sys.argv[0])

NSAMPLES = int(4 * 1024)
ROWS = 32
N_ZBOARD = 2*4*NSAMPLES

# соответствует af.h:LOGROWSLEN
N_DSP = 2*7*NSAMPLES

X = NSAMPLES

# растяжение оси X
#XSCALE1 = 64 
XSCALE1 = 16 
XSCALE2 = 16 
Y = 32768 

# коэффициенты усиления для каждого лога - для нормировки отображения

K1_SIG = 1.0/100.0
K2_OSIG = 1.0
K3_REF = 1.0/100.0
K4_OREF = 1.0
# не требует усиления, поскольку max mul = 32767 * 32767 / 65535 = 16383
K2_MUL = 1.0
K3_DSP = 1.0/300.0
K4_SDSP = 30.0
K5_DOT = 1.0/100.0
K6_AMB = 100.0

'''
Отрисовка окон и виджетов
'''

app = QtGui.QApplication([])
mw = pg.GraphicsWindow(title="SM")
mw.resize(1200,800)
mw.setWindowTitle('Signal Monitor')
pg.setConfigOptions(antialias=True)

# zboard plots

if MODE != "dsp":
	#pw1 = mw.addPlot(title="ZBOARD: afilter in(I)/out(I)", y=np.random.normal(size=100))
	pw1 = mw.addPlot(title="ZBOARD: afilter in(I)/out(I)")
	pw1.setLabel('left', 'int/short', units='V')
	pw1.setLabel('bottom', 'Time', units='s')
	pw1.setXRange(0, X/XSCALE1)
	pw1.setYRange(-Y, Y)
	pw1.addLegend(size=(20, 20), offset=(30, 30))
	# sig_i
	p11 = pw1.plot(pen='r', name='sig I')
	# osig_i
	p12 = pw1.plot(pen='g', name='osig I')
	# ref_i
	p13 = pw1.plot(pen='b', name='ref I')

	pw3 = mw.addPlot(title="ZBOARD: ref(I) int -> short")
	pw3.setLabel('left', 'int/short', units='V')
	pw3.setLabel('bottom', 'Time', units='s')
	pw3.setXRange(0, X/XSCALE1)
	pw3.setYRange(-Y, Y)
	pw3.addLegend(offset=(30, 30))
	# ref_i
	p31 = pw3.plot(pen='r', name='ref I')
	# oref_i
	p32 = pw3.plot(pen='g', name='oref I')

if MODE == "both":
	mw.nextRow()

# DSP plots

if MODE != "zboard":
	pw2 = mw.addPlot(title="DSP: sig, conj(ref), sig*conj(ref)(t<ROWS)")
	pw2.setLabel('left', 'int/short', units='V')
	pw2.setLabel('bottom', 'Time', units='s')
	pw2.setXRange(0, X/XSCALE1)
	pw2.setYRange(-Y, Y)
	pw2.addLegend(size=(20, 20), offset=(30, 30))
	# s_i
	p21 = pw2.plot(pen='r', name='sig I')
	# r_i
	p22 = pw2.plot(pen='g', name='conj(ref) I')
	# mul_i
	p23 = pw2.plot(pen='b', name='mul(t<ROWS) I')

	pw4 = mw.addPlot(title="DSP(t): FFT32, FFT16, dot, af")
	pw4.setLabel('left', 'int/short', units='V')
	pw4.setLabel('bottom', 'Time', units='s')
	pw4.setXRange(-X/XSCALE2/2, X/XSCALE2/2)
	pw4.setYRange(-Y, Y)
	pw4.addLegend(offset=(30, 30))
	# dsp_i
	p41 = pw4.plot(pen='r', name='fft32(t<ROWS)')
	# sdsp_i
	p42 = pw4.plot(pen='g', name='fft16(t<ROWS)')
	# dot_i
	p43 = pw4.plot(pen='b', name='dot(t<ROWS)')
	# amb_i
	p44 = pw4.plot(pen='w', name='af(t<ROWS)')

mw.show()

print("Frames", FRAMES, "Mode:", MODE, "zport:", PORT, "dport:", PORT+1, "NSAMPLES:", NSAMPLES, "X:", X)
#print(data[:16])

# FIXME: это удалить
'''
import sys
if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
    QtGui.QApplication.instance().exec_()
'''

'''
Подготовка multiprocessing
'''

# количество одновременно слушающих tcp портов и
# соответственно параллельно слушающих процессов
# в конфигурациях НЕ MODE=both неактивные сокеты пропускаются
PROCS = 2

tcp = np.empty((PROCS), dtype=object)

# tcp[0] для zboard
# tcp[1] для dsp
for i in range(PROCS):
	tcp[i] = sc.socket(sc.AF_INET, sc.SOCK_STREAM)
	tcp[i].bind(("0.0.0.0", PORT + i))
	tcp[i].listen(5)

pool = Pool(processes = PROCS)

# структура pmap:
# [[0, n1],[1, n2], ...]
# где 0..1 - номера процессов
# nX - количество байт которое ожидается принять для каждого процесса
pmap = np.array([[0, N_ZBOARD],[1, N_DSP]], dtype=int)
