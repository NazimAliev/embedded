#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import os,sys
import numpy as np
import socket

# 3 km = 10 us = 20 samples
# 60 km/h = 150 Hz

'''
N, dfnum должны соответствовать аналогичным параметрам в cv_app.h для DSP=True
'''

ktf = 0.01 
K = 2000 

# масштаб K необходимо юстировать в зависимости от коэффициента ktf
# в случае использования адаптивного фильтра
#ktf = 96. * 1 
#K = 125. / 1 
# частота выборки в секунду, определяется приемником
srate = 1024000.
#srate = 1024000.

# коэффициент децимации
dec = 1 
# количество фреймов
frames = 2 
# параметры адаптивного фильтра
#taps = 64
#slices = 512
rows = 32

# ==============================================
# количество отсчетов для одного фрейма после децимации
# разрешение по времени
N = 1024

# разрешение по времени
# период между выборками по времени в секундах
# расстояние между выборками по времени
T = 1./srate
#TDs = dec * Ts
# длительность пачки в секундах
#tmax = TDs * N

taps = 32 
slices = (int)((N * frames) / taps)

'''
Параметры имитатора
'''

dopler =20000 / dec
# имитация задержки по времени, samples
# на картинке задержка независима от dec, хотя масштаб оси меняется
delay = 16 * dec 

'''
Инициализация UDP для DSP версии AF
'''

#server = 'cm-debian.vcs.local'
#server = '192.168.0.6'
server = '192.168.77.2'
#server = '172.16.165.152'
#server = 'bbxm.vcs.local'
#==================
port = 32000

headsize = 4*4
msgsize_sig = (N+rows)*2*2  # *2: double because IQ
msgsize_ref = N*2*2  # *2: double because IQ
MTU = 1024

frags_sig = (int)(msgsize_sig/MTU)
if frags_sig == 0:
    frags_sig = 1

frags_ref = (int)(msgsize_ref/MTU)
if frags_ref == 0:
    frags_ref = 1

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

