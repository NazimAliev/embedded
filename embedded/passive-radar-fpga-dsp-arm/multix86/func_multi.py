#!/usr/bin/python3
# -*- coding: utf8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import numpy as np
import numpy.ma as ma 
import init_multi as im 

'''
====================================
'''

# TODO: разложить на два потока

# возвращает параметрическое описание эллипса
# x0, y0: координаты центра эллипса
# a, b: размер осей
# k: коэффициент наклона, в радианах

def ellipseCreate(x0, y0, a, b, k):
  #x = np.cos(np.linspace(0, (2+0.1*i)*np.pi, 1000))
  #y = np.sin(np.linspace(0, 4*np.pi, 1000))
  t = np.linspace(0, 2*np.pi, im.T)

  x = x0 + a * np.cos(t) * np.cos(k) - b * np.sin(t) * np.sin(k)
  y = y0 + b * np.sin(t) * np.cos(k) + a * np.cos(t) * np.sin(k)
  return x, y

# чтение fifo CFAR, распаковка битов в 8 слов 0/1
def getFifoCfar():
  fifo = im.fifo.read(im.CFAR_SIZE)
  print("read CFAR FIFO len", len(fifo))
  udata = bytearray(fifo)

  # каждый байт fdata превращается в 8 слов с содержимым 0 или 1
  # размер data CFAR_SIZE в 8 раз больше чем fdata = NSAMPLES*ROWS
  fdata = np.unpackbits(udata)

  # индикация количества целей в массиве, должно совпадать с логом DSP
  print("FA count", np.count_nonzero(fdata))

  # CFAR пока еще линейный массив
  data16 = np.array(fdata).astype(np.uint16)

  # образовали меньший в FACTOR раз массив, в котором
  # каждая ячейка получена суммированием последовательных ячеек исходного массива в количестве FACTOR
  data = data16.reshape(im.FNSAMPLES * im.ROWS, im.FACTOR) # FACTOR столбцов
  data = np.sum(data, axis=1) # строки просуммированы, остался один столбец размером FNSAMPLES*ROWS 

  # столбец преобразуется в картинку для отображения
  # горизонталь x = ROWS: t
  # вертикаль y = FNSAMPLES: f
  data = data.reshape((im.ROWS, im.FNSAMPLES))
  #data = np.transpose(data)

  # fftshift
  # нормализация картинки: 
  # положительные частоты будут отсчитываться от средней горизонтальной линии вверх
  # отрицательные частоты будут отсчитываться от средней горизонтальной линии вниз 
  im.cfar_ds[:, im.FNSAMPLES/2 : im.FNSAMPLES] = data[:, 0 : im.FNSAMPLES/2]
  im.cfar_ds[:, 0 : im.FNSAMPLES/2] = data[:, im.FNSAMPLES/2 : im.FNSAMPLES]
  return im.cfar_ds

# чтение fifo AF
def getFifoAf(n):
  fifo = im.fifo.read(n)
  print("read AF FIFO len", len(fifo))
  udata = bytearray(fifo)
  data16 = np.frombuffer(udata, 'u2')
  data = data16.reshape(im.ROWS, im.NSAMPLES)
  im.af_ds[:, im.NSAMPLES/2 : im.NSAMPLES] = data[:, 0 : im.NSAMPLES/2]
  im.af_ds[:, 0 : im.NSAMPLES/2] = data[:, im.NSAMPLES/2 : im.NSAMPLES]
  # метка отсчета
  #im.af_ds[0, im.NSAMPLES/2] = 1000

  return im.af_ds

# ассоциация треков, для каждого вызова с параметром новой отметки:
# начинается новый трек, или отметка ассоциируется с существующим треком
# апдейт треков на время жизни

# plot требует массив для отображения даже для одной точки
xx = np.zeros((1))
yy = np.zeros((1))

def plot2track(x, y):
  # номер трека
  # FIXME пока работаем с нулевым
  tn = 0

  # текущий трек задан как list of list, преобразуем в numpy array
  tx = np.array(im.trackx[tn])
  ty = np.array(im.tracky[tn])
  #print("")
  #print("Track #0 before:", np.around(tx + 1j*ty))

  # при пустом массиве следующая часть будет давать ошибку - избегаем этого
  if len(tx) != 0:

    # отображаем текущий трек квадратиками, соединенными линиями
    im.mem.setData(tx, ty)

    # alpha beta filter по двум координатам
    # на выходе фильтра - предсказанная оценка следующей координаты
    fcx = abFilter(tx)
    fcy = abFilter(ty)
    #print("Estimate next:", np.around(fcx + 1j*fcy))

    # оценку - в массив единичной длины для отображения
    xx[0] = fcx
    yy[0] = fcy

    # предсказанная оценка отрисовывается крестиком
    im.future_est.setData(xx, yy)

    #print("Real next:", np.around(x[-1] + 1j*y[-1]))

  # отображение набора координат поступивших целей
  xx[0] = x[tn]
  yy[0] = y[tn]

  # новая отметка отображается белым квадратиком
  im.future_real.setData(xx, yy)

  # новая отметка добавляется к треку
  im.trackx[tn].append(x[tn])
  im.tracky[tn].append(y[tn])

# TODO: разложить на два потока
# нахождение точек пересечения кривых 1, 2
def ellipseCross(x1, y1, x2, y2):
  # все сочетания двух эллипсов по каждой ординате 
  cx = np.transpose([np.tile(x1, len(x2)), np.repeat(x2, len(x1))])
  cy = np.transpose([np.tile(y1, len(y2)), np.repeat(y2, len(y1))])

  # разница по каждой оси
  dx = cx[:,0] - cx[:,1]
  dy = cy[:,0] - cy[:,1]

  # расстояние между двумя точками принадлежащими двум эллипсам
  # во всех сочетаниях существующих точек между собой
  d = np.hypot(dx, dy)

  # индексы для пересечений, где расстояние меньше порога
  idx = np.where(d < 0.4)

  # выявление близко стоящих индексов: разница будет или 1, или T, или T+1
  idx_delta = np.roll(idx, -1) - idx
  mask = np.in1d(idx_delta, im.test)
  idx_masked = ma.masked_array(idx, mask)
  # idx_data уже не содержит индексов рядом стоящих точек
  idx_data = idx_masked.compressed()

  # координаты точек пересечения как среднее от двух точек разных эллипсов
  #x = (cx[:,0] + cx[:,1]) / 2
  #y = (cy[:,0] + cy[:,1]) / 2
  x = (cx[idx_data,0] + cx[idx_data,1]) / 2
  y = (cy[idx_data,0] + cy[idx_data,1]) / 2
  #print("===")
  #print(idx_delta)
  #print(np.around(np.vstack((x, y)).T))

  # np.dstack(numpy.meshgrid(x, y)).reshape(-1, 2)
  return x, y

# alpha beta фильтр
# y[N] является предсказанным значением
def abFilter(x):
  dt = 0.01  # шаг по времени
  xk_1 = 0.0  # предыдущее значение x
  vk_1 = 0.0  # предыдущее значение v
  a = 0.95    # alpha
  b = 0.3   # beta

  xk = 0.0    # текущее значение x
  vk = 0.0    # текущее значение v
  rk = 0.0    # текущее значение ошибки

  memlen = 8  # глубина памяти фильтра

  N = x.size
  if N > memlen:
    start = N - memlen
  else:
    start = 0

  y = np.zeros(N, dtype=float)

  for t in range(start, N):
    # прогноз текущего значения исходя из прошлых значений
    xk = xk_1 + ( vk_1 * dt )
    #print "xk=",xk,"xk_1",xk_1,"vk_1*dt", vk_1*dt
    vk = vk_1

    # сравнение прогноза с текущим значением: сигнал ошибки
    rk = x[t] - xk

    # коррекция
    xk = xk + a * rk
    vk = vk + ( b * rk ) / dt

    # перезапись выхода в прошлое значение
    xk_1 = xk
    vk_1 = vk

    #y[t] = xk
    #print "x:", x[t].astype(int), "xk:", xk.astype(int), "vk:", vk, "rk:", rk.astype(int)

  # прогноз на будущее
  return xk + ( vk * dt )
  #return y

