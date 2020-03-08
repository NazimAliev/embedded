#!/usr/bin/python3

# -*- coding: utf-8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


"""
Главная программа монитора
"""

from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import init_mon as im
import func_mon as fm

def updateData():
	global i
	#y, x = fm.rand(100)
	print(i)
	# ось X размерностью в количество выборок
	
	if True:
		# боевой режим
		buf = im.pool.map(fm.tcp_wrap, im.pmap)
		print("\t", i)
		buf_zboard = buf[0]
		assert(buf_zboard.size == im.N_ZBOARD)
		buf_dsp = buf[1]
		assert(buf_dsp.size == im.N_DSP)
	else:
		# тестовый режим без TCP и платы DSP
		buf_zboard = np.arange(im.N_ZBOARD, dtype='u1')
		buf_dsp = np.arange(im.N_DSP, dtype='u1')

	if im.MODE != 'dsp':
		fm.showZboard(buf_zboard)
	if im.MODE != 'zboard':
		fm.showDsp(buf_dsp)
	
	if i < im.FRAMES - 1:
		i += 1
		t.start(800)

# счетчик фреймов
i = 0

## Start a timer to rapidly update the plot in pw
t = QtCore.QTimer()
t.timeout.connect(updateData)
t.setSingleShot(True)
t.start(0)
#updateData()

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
		print("is going to exit...")
		im.app.deleteLater()
		exit()

