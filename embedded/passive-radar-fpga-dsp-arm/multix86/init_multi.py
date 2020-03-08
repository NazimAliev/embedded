#!/usr/bin/python3
# -*- coding: utf8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import os,sys
from pyqtgraph.Qt import QtCore, QtGui
import pyqtgraph as pg
import pyqtgraph.opengl as gl
import numpy as np

'''
графическое представление CFAR и AF
рендеринг мультистатической секции
'''
# значения должны быть точно такими же как в ../af.h
NSAMPLES = 4 * 1024
ROWS = 32
# будет ли отображаться матрица AF
AF_UDP=1

CFAR_SIZE = int((NSAMPLES * ROWS) / 8)
AF_SIZE = int((NSAMPLES * ROWS))

# поскольку при размере NSAMPLES>24 биты CFAR уже не помещаются в экран,
# выполняется сжатие по оси NSAMPLES с данным фактором
# коэффициент показывает сколько соседних ячеек будет объединено
FACTOR = 16 
FNSAMPLES = int(NSAMPLES / FACTOR)
cfar_ds = np.zeros((ROWS, FNSAMPLES), dtype=np.uint16)
af_ds = np.zeros((ROWS, NSAMPLES), dtype=np.uint16)

# треки задаются как list of list, чтобы иметь строки переменной длины
TRACKS = 4
trackx = []
tracky = []
for i in range(TRACKS):
  trackx.append([])
  tracky.append([])

# количество точек параметризации
T = 800

# если разница между индексом и предыдущим индексом соответствует числам из этого массива,
# то это рядом стоящие точки которые могут быть отброшены
test = np.array([1, T-1, T, T+1]).astype(np.int32)

# количество циклов которое проработает приложение
if len(sys.argv) != 2:
  frames = 1
else:
  frames = int(sys.argv[1])

print("Python compiled options:\n\tNSAMPLES: %d\n\tROWS: %d\n\tAF_UDP: %d\n\tFACTOR: %d" % (NSAMPLES, ROWS, AF_UDP, FACTOR))
print("Frames:", frames)

# входной fifo тракт, данные для fifo готовит приложение multix86
# которое в свою очередь принимает их по UDP в разных потоках со всех каналов
FIFO = '/tmp/af.fifo'
try:
  os.mkfifo(FIFO)
except OSError:
  pass

# нужен ключ -b иначе будет восприниматься как текстовый файл и проблемы с кодеками!
fifo = open(FIFO, 'rb')

'''
====================================
инициализация графической подсистемы
используется OpenGL
====================================
'''
app = QtGui.QApplication([])

''' WIN AIR '''
win_air = pg.GraphicsLayoutWidget()
win_air.setFixedSize(600, 600)
win_air.move(10, 10)
win_air.setWindowTitle(u'Пассивный радиолокатор \"Ceramic LW 2700\" www.linkware.ru')
win_air.show()

plt_air = win_air.addPlot(title=u'Индикатор мультистатической секции')
x = np.linspace(0, 2*np.pi, 30)
y = np.linspace(0, 2*np.pi, 30)
ellipse1 = plt_air.plot(x, y, pen='#8f8f8f')
ellipse2 = plt_air.plot(x, y, pen='#8f8f8f')
cross = plt_air.plot(x, y, pen=None, symbol='o', symbolPen=None, symbolSize=10, symbolBrush=(0.2))

# текущий трек отображается полыми квадратиками, соединенными линиями
mem = plt_air.plot(x, y, pen=(0.4), symbol='s', symbolPen=(0.4), symbolSize=10, symbolBrush=None)

# предсказанная оценка положения цели отображается крестиком
future_est = plt_air.plot(x, y, pen=None, symbol='+', symbolPen=(0.8), symbolSize=10, symbolBrush=None)

# новая отметка цели отображается белым квадратиком
future_real = plt_air.plot(x, y, pen=None, symbol='s', symbolPen=(0.8), symbolSize=10, symbolBrush=None)
plt_air.showGrid(x=True, y=True)
plt_air.setAspectLocked(True)
#view2.setAspectLocked(True)

''' WIN TARGET '''
win_target = pg.GraphicsLayoutWidget()
win_target.setFixedSize(600, FNSAMPLES)
win_target.move(800, 10)
win_target.setWindowTitle(u'Отметки целей: x:time, y:freq')
win_target.show()

win_target.addLabel(u'Канал 1: 123.7 MHz', row=0, col=0)
win_target.addLabel(u'Канал 2: 107.5 MHz', row=0, col=1)

view1 = win_target.addViewBox(row=1, col=0)
view2 = win_target.addViewBox(row=1, col=1)

## lock the aspect ratio so pixels are always square
#view1.setAspectLocked(True)
#view2.setAspectLocked(True)

img1 = pg.ImageItem(border='#8f8f8f')
view1.addItem(img1)
img2 = pg.ImageItem(border='#8f8f8f')
view2.addItem(img2)

## Set initial view bounds
view1.setRange(QtCore.QRectF(0, 0, ROWS, FNSAMPLES))
view2.setRange(QtCore.QRectF(0, 0, ROWS, FNSAMPLES))

if AF_UDP == 1:
  ''' WIN GL '''
  ## Create a GL View widget to display data
  win_gl = gl.GLViewWidget()
  #win_gl.setFixedSize(800, 800)
  win_gl.move(800, 10)
  win_gl.setWindowTitle(u'Функция неопределенности сигнала')
  win_gl.show()

  win_gl.setCameraPosition(distance=50)

  ## Add a grid to the view
  grid_gl = gl.GLGridItem()
  # задает размер сетки снизу независимо от цветной области
  grid_gl.scale(4,8,1)
  grid_gl.setDepthValue(10)  # draw grid after surfaces since they may be translucent
  win_gl.addItem(grid_gl)

  cols_gl = ROWS
  rows_gl = NSAMPLES 
  # linspace задает размер цветной области
  x_gl = np.linspace(-ROWS, ROWS, cols_gl).reshape(cols_gl,1)
  y_gl = np.linspace(-NSAMPLES/256, NSAMPLES/256, rows_gl).reshape(1,rows_gl)

  d_gl = (x_gl**2 + y_gl**2) * 0.1
  d2_gl = d_gl ** 0.5 + 0.1

  ## create a surface plot, tell it to use the 'heightColor' shader
  ## since this does not require normal vectors to render (thus we 
  ## can set computeNormals=False to save time when the mesh updates)
  plot_gl = gl.GLSurfacePlotItem(x=x_gl[:,0], y = y_gl[0,:], shader='heightColor', computeNormals=False, smooth=False)
  plot_gl.shader()['colorMap'] = np.array([0.2, 2, 0.5, 0.2, 1, 1, 0.2, 0, 2])
  plot_gl.scale(0.5,0.5,0.01)

  # сдвигает диаграмму к углу квадранта
  #plot_gl.translate(10, 10, 0)
  win_gl.addItem(plot_gl)

'''
====================================
'''
