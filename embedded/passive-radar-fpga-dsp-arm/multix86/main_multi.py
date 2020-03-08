#!/usr/bin/python3
# -*- coding: utf8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import init_multi as im
import func_multi as fm 

'''
BEGING PROGRAM
'''

# функция отрисовки изображения, использует convert() для получения данных из fifo
def showTrack(i):
  
  x1, y1 = fm.ellipseCreate(20+5*i, 30, 50, 70-3*i, 0.05*i)
  im.ellipse1.setData(x1, y1)
  x2, y2 = fm.ellipseCreate(40-5*i, 20, 40, 80+3*i, 0.07*i)
  im.ellipse2.setData(x2, y2)
  
  tx, ty = fm.ellipseCross(x1, y1, x2, y2)
  '''
  tx = np.zeros((1))
  ty = np.zeros((1))
  tx[0] = 10 + i*5
  ty[0] = 20 + i*6
  '''
  fm.plot2track(tx, ty)
  im.cross.setData(tx, ty)

def showCfar(i):

  data_target = fm.getFifoCfar()
  print("\tdata_target", data_target.shape)
  data_target[:, im.FNSAMPLES/2] = 1 

  im.img1.setImage(data_target)
  im.img2.setImage(data_target)

def showAf(i):

  data_af = fm.getFifoAf(2*im.AF_SIZE)
  print("\tdata_af", data_af.shape)
  #Z = np.sin(i*im.d_gl) / im.d2_gl
  #im.plot_gl.setData(z=Z)
  im.plot_gl.setData(z=data_af)


def updateData():
  
  global i
  showTrack(i)
  #showCfar(i)
  if im.AF_UDP == 1:
    print("[%d]" % (i))
    showAf(i)

  # программирование таймера
  # запуск этой же функции снова через 1 сек
  #im.QtCore.QTimer.singleShot(0.1, updateData)
  if i < im.frames - 1:
    i += 1
    t.start(100)


i = 0
t = QtCore.QTimer()
t.timeout.connect(updateData)
t.setSingleShot(True)
t.start(0)

if __name__ == '__main__':
  
  print("Start")
  # запуск в первый раз чтобы подхватить таймер
  #updateData()

  ## Start Qt event loop unless running in interactive mode.
  import sys
  if (sys.flags.interactive != 1) or not hasattr(mi.QtCore, 'PYQT_VERSION'):
      im.QtGui.QApplication.instance().exec_()

  im.fifo.close()
  exit(0)


'''
'''
