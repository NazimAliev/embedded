#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

'''
Визуальное отображение результата работы DSP accelerator
'''
import os,sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sc
import socket
from time import sleep

import pclvars as v
import libutils as ut

IMITATOR = False
USEADAPT = False 
FILEDATA = False
'''
Считывание входных I/Q данных с файла
Данные считываются однократно, количество I/Q выборок count выбирается так чтобы обеспечить
отображение фреймов в количестве frames:

  count=N*dec*frames -  количество выборок для одного фрейма до децимации,
  после децимации уменьшается в dec раз и становится равным N
'''

#assert v.delay < v.max_delay
if FILEDATA:
  ### Выборка массива из файла длиной count ###
  # лишний фрейм потому что последний sig выйдет за границу массива справа на величину rows
  Sfile = ut.readv_iq('rtl_1024MHz_512ks.dat', 1000, v.N * v.dec * (v.frames+1))
  Sfile = v.K * Sfile
  # помеха: сигнал от базовой станции
  sig = Sfile
  ref = Sfile
else:
  ss = v.N * v.dec * (v.frames+1)
  tt = np.arange(ss) 
  Sfile = v.K * np.exp(-1j*2*200*np.pi*tt/ss) 
  ref = Sfile
  tar = v.K * np.exp(-1j*2*300*np.pi*tt/ss)/2.0
  sig = tar + ref 
  #sig = Sfile 
# 

if IMITATOR:
  # имитация отраженного от цели сигнала: сдвиг по времени на delay и по доплеровской частоте на dopler
  #Stf = 
  # сигнал отраженный от цели плюс помеховый сигнал FM
  sig = ut.target(Sfile, v.delay, v.dopler) + v.ktf * ref

#sig.real = np.random.random_integers(-4*v.K, 4*v.K, Sfile.size)
#sig.imag = np.random.random_integers(-4*v.K, 4*v.K, Sfile.size)

#t = np.linspace(-4, 4, 8)
#sig[:8] = v.K * np.exp(1j * 2*np.pi * t/2)

print('')
print('=== ****** IMITATOR ****** ===')
print('Target time delay, samples:', v.delay)
print('Target dopler shift, Hz', v.dopler)
print('Signal level, -dB:', 20.0*np.log10(1.0/v.ktf))
print('=== ********************* ===')
print('')

print('Received data, complex samples: ', Sfile.size)
print('')
print('dec:', v.dec)
print('N:', v.N)
print('rows:', v.rows)
print('N+rows:', v.N+v.rows)

BANDFILTER = False
if BANDFILTER:
  # пропускаем через ФНЧ для ограничения помех по спектру
  taps = ut.lband(v.srate)
  sig = sc.lfilter(taps, 1.0, sig)
  ref = sc.lfilter(taps, 1.0, ref)

# децимация: уменьшение размера выборки с ND*frames до N*frames
sig = ut.firfilter(sig, v.dec)
ref = ut.firfilter(ref, v.dec)

if USEADAPT:
  # смотрим первые N выборок с адаптивным фильтром
  # адаптивный фильтр
  # подавление сигнала FM станции в суммарном сигнале цели

  # выход фильтра соответствует размеру выборки N потому что N=taps*slices
  l = v.N * v.frames
  sig[:l].real, ref[:l].real = ut.afilter(ref.real, sig.real)
  sig[:l].imag, ref[:l].imag = ut.afilter(ref.imag, sig.imag)

'''
До этого момента все операции были с целым файлом (массивом длиной в файл)
Далее, в цикле идет поблочная выборка из файла и обрабатывается каждый блок
'''

# готовим блок для считывания порции данных N в цикле
sigBlock = np.zeros(v.N + v.rows, dtype=complex)
refBlock = np.zeros(v.N, dtype=complex)
print('sig, ref size:', sig.size, ref.size)

offset = 0	# смещение во входном массиве для выборки блока

# цикл по входному массиву с поблочной выборкой и обработкой
for i in range(v.frames):

  print('')
  print('Frame:', i, 'offset:', offset, 'sig/ref size:', sig.size, ref.size)
  # выбираем из всего входного массива очередной блок размера N для обработки
  sigBlock = sig[offset: offset + v.N + v.rows]
  refBlock = ref[offset : offset + v.N]
  #sigBlock = ut.norm(sigBlock)
  #refBlock = ut.norm(refBlock)
  print(sigBlock[0:16])
  print('')
  print(refBlock[0:16])

  # ==========================
  # Вызываем DSP сервер по UDP
  # ==========================

  '''
  Длина SDA может быть избыточна, поскольку она больше N на величину taps (Nsig),
  а для передачи по UDP ожидается строго N + rows
  Поэтому массив усекается до длины N + rows
  '''
  print(sigBlock.size, refBlock.size)
  ut.send_udp(sigBlock, mode=0)
  ut.send_udp(refBlock, mode=1)
#  exit(0)
  offset += v.N

# 
v.client_socket.close()

'''
GRAPHIC
'''
sleep(2)

filename1 = '/home/legolas/dev/dsp/bbxm/targetNFS/root/af_app/nefilter1.bin'
filename2 = '/home/legolas/dev/dsp/bbxm/targetNFS/root/af_app/nefilter2.bin'

# >i4 32bit int, big endian
# <i4 32bit int, little endian

raw1 = np.fromfile(filename1, dtype=np.float32)
raw2 = np.fromfile(filename2, dtype=np.float32)
print(raw1[:16])
print(raw2[:16])

frame = 1024/8

fig = plt.figure(1)

ax1 = fig.add_subplot(311)
ax1.plot(tar[:frame], label='Target')
ax1.plot(ref[:frame], label='Ref')
ax1.legend()

ax2 = fig.add_subplot(312)
ax2.plot(raw1[:frame], label='1st')
ax2.legend()

ax3 = fig.add_subplot(313)
ax3.plot(raw2[:frame], label='2nd')
ax3.legend()

plt.show()
