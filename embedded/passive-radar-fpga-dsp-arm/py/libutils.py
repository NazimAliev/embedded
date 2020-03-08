#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import os,sys

import pclvars as v
import numpy as np
import scipy.signal as sc
import scipy.linalg as li
import time

### чтение count комплексных чисел из файла filename начиная с offset числа
### возвращает комплексный вектор
def read_iq(filename, offset, count):
    # prepare signal from file
    f = open(filename, 'rb')
    f.seek(2*offset)
    udata = bytearray(f.read(2*count))
    f.close

    data = np.zeros(len(udata), np.byte)
    for i in range(len(udata)):
        data[i] = udata[i] - np.byte(127)
    Z = np.zeros(count, dtype=complex)

    for i in range(count):
        Z[i] = complex(data[i*2], data[i*2+1])
    return Z

# версия read_iq без цикла for
def readv_iq(filename, offset, count):
    # prepare signal from file
    f = open(filename, 'rb')
    f.seek(2*offset)
    udata = bytearray(f.read(2*count))
    f.close
    data = np.array(udata, dtype=np.byte) - np.byte(127)

    y = data[0::2] + 1j * data[1::2]
    return y
    
def target(Sfile, delay, dopler):

    count = Sfile.size
    St = np.zeros(delay, dtype=complex)

    # внесение задержки по времени delay= количеству задержанных выборок
    # сигнал запаздывает относительно начала массива,
    # что соответствует положительной задержке
    St = np.append(St, Sfile[:count-delay])

    # внесение частотного доплеровского сдвига
    # n = srate выборок нужно чтобы длительность сигнала была 1 сек
    # один период изменения за 1 сек соответствует частоте 1 Гц
    # при t = srate 2*pi*t/srate будет таким оборотом
    # если выборок не srate а count = n*srate то нужно 2*pi*n*t/srate таких оборотов 
    # n = count / srate = count * T
    # требуемое количество оборотов 2*n*t*T = 2*pi*count*T*t*T
    t = np.linspace(-count/2, count/2, count)
    exf = np.exp(1j * 2. * np.pi * dopler * t * v.T)
    Stf = St * exf

    return(Stf)

def firfilter(Sin, dec):
    if dec > 1:
        Sout = sc.decimate(Sin, q=dec, ftype='fir')
    else:
        Sout = Sin
    return Sout

'''
вычисление коэффициентов отражения p и синтезирующего фильтра h
на основе входной выборки x
для коэффициентов h дополнительно нужна выборка желаемого сигнала h
'''
def hcalc(x, d):
  # самостоятельное вычисление коэффициентов
  L = len(x)
  #rxx = np.array(lz.acorr(x))
  rxx = np.correlate(x, x, "full")[L-1:]
  Rxx = li.toeplitz(rxx)
  Pdx = np.correlate(d, x, "full")[L-1:]

  p = np.zeros(L)
  a = np.zeros((L, L))
  sig = np.zeros(L)
  h = np.zeros(L)

  p[0] = rxx[1] / rxx[0]
  a[0,0] = p[0]
  sig[0] = rxx[0] * (1 + np.square(p[0]))
  h[0] = Pdx[0] / rxx[0]
  h[1] = (Pdx[1] - a[0,0] * Pdx[0]) / sig[0]

  for k in range(1, L-1):
    m1 = Rxx[k+1, 1:k+1]
    m2 = a[0:k, k-1]
    Ra = np.sum(m1 * m2)
    p[k] = (rxx[k+1] + Ra) / sig[k-1]
    a[k, k] = p[k]
    a[0:k, k] = a[0:k, k-1] - p[k] * a[k-1 ::-1, k-1]
    sig[k] = sig[k-1] * (1-np.square(p[k]))
    m3 = np.sum(Pdx[0:k+1] * a[k ::-1, k])
    h[k+1] = (Pdx[k+1] + m3)/sig[k]
  p = p[:len(p)-1]
  return p, h

''' 
Решетчатый фильтр
'''
def lattice(x, p, h):

  taps = len(p)
  f = x
  b = x
  fir = np.multiply(x, h[0])
  for k in range(taps):

    b1 = np.append([0], b[0 : len(x) - 1])
    temp = f - p[k] * b1
    b = b1 - np.multiply(f, p[k])
    f = temp
    fir = fir + b * h[k+1]
  return f, b, fir

'''
СОБСТВЕННАЯ версия
Фильтр извлекает полезный сигнал x из входной выборки s = x + ref,
где ref - сигнал помехи

Алгоритм:
 - решетчатый фильтр настраивает коэффициенты 'p' в соответствии с ref;
 - 'b' массив решетчатого фильтра подается на вход FIR фильтра;
 - коэффициенты 'h' FIR фильтра настраиваются в соответствии с s;
 - на выходе FIR фильтра формируется оценка ref, которая вычитается из входной выборки s/
 - в результате чего на выходе фильтра присутствует сигнал x

порядок фильтра
taps

столько раз будут пересчитываться коэффициенты фильтра
нарезка входной выборки по слайсам
в каждом слайсе taps выборок
slices

размер выборки входного сигнала
N = taps * slices

Выход фильтра: полезный сигнал x и ошибка
'''

def afilter(ref, d):

  taps = v.taps
  slices = v.slices

  # предсказанный сигнал
  yfir = np.empty(0)
  # ошибка фильтра
  ye = np.empty(0)

  # цикл по слайсам
  for i in range(slices):

    i1 = i * taps
    i2 = (i+1) * taps
    sref = ref[i1 : i2]
    sd = d[i1 : i2]
    #print("MY#%02d %02d --> %02d" % (i, i*taps, (i+1)*taps-1))

    p, h = hcalc(sref, sd)
    f, b, fir = lattice(sref, p, h)
    e = b
    # оценка ref
    yfir = np.append(yfir, fir)
    ye = np.append(ye, e)

  return d[0 : taps*slices] - yfir, yfir

# посылка sig или ref по UDP
def send_udp(ar, mode):
    # >i4 32bit int, big endian
    # <i4 32bit int, little endian

    # фрейм формируется здесь а не однократно снаружи потому что может меняться от посылке к посылке
    # структура фрейма: четыре unsigned int32, по краям магические числа
    # 575757 + #Op + CRC(msg) + 919191
    # #Op: =0 for FFT, =1 for IFFT
    head = np.empty(4, dtype='<u4')
    head[0] = np.uint32(575757)
    head[1] = np.uint32(9)
    head[2] = np.uint32(0)
    head[3] = np.uint32(919191)
    headbyte = np.ndarray(shape=v.headsize, dtype='u1', buffer=head)

    # посылаем фрейм 
    v.client_socket.sendto(headbyte, (v.server, v.port))
    #print('sent head:', len(headbyte))

    # в режиме mode=0 посылаем массив sig на rows shorts длиннее
    if mode == 0:
        N = v.N + v.rows
        msgsize = v.msgsize_sig
        frags = v.frags_ref
    else:
        N = v.N
        msgsize = v.msgsize_ref
        frags = v.frags_ref

    # место под short IQ
    msg = np.empty(N * 2, dtype='<i2')

    # XXX порядок следования - ImRe !!!
    msg[0::2] = np.int16(ar.imag)
    msg[1::2] = np.int16(ar.real)

    msgbyte = np.ndarray(shape=msgsize, dtype='u1', buffer=msg)

    # данные посылаются и принимаются частями
    # для обеспечения < MTU

    x = int(msgsize/frags)
    for i in range(frags):
        # посылаем массив для FFT частями
        n = v.client_socket.sendto(msgbyte[i*x:(i+1)*x], (v.server, v.port))
        #print('#', i+1, n, 'sent msg from', i*x, 'to', (i+1)*x)
        #time.sleep(0.01) # Time in seconds.
    print('Sent msg mode', mode, frags, 'frags', x, 'bytes')
    time.sleep(0.2) # Time in seconds.


'''
Синтез FIR фильтра
'''
def lband(srate):

  nyq_rate = srate / 2.0
  width = 50000.0/nyq_rate
  ripple_db = 20.0
  Nf, beta = sc.kaiserord(ripple_db, width)
  cutoff_hz = 100.0
  taps = sc.firwin(Nf, cutoff_hz/nyq_rate, window=('kaiser', beta))
  return taps

'''
Нормировка комплексного массива так, чтобы каждая из составляющих
Re, Im не превысила short (16 bit)
'''

def norm(ar):

  arf = ar.real
  arf = np.append(arf, ar.imag)
  armax = np.amax(arf)
  return (32767. * ar) / armax
