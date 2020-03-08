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
import socket
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from time import sleep

# === variables ===
N = 1024 
ROWS = 32 
K = 30000	# с увеличением размера выборки амплитуду надо делать меньше
#SERVER = 'cm-debian.vcs.local'
#SERVER = '192.168.0.73'
#SERVER = '172.16.165.152'
SERVER = 'bb1'
#==================
PORT = 32000

FRAMESIZE = 4*4
WORD_SIZE = 2    # short

MTU = 1024

SEND_MSGSIZE_SIG = (N+ROWS)*2*2  # *2: double because IQ
SEND_MSGSIZE_REF = N*2*2  # *2: double because IQ

SEND_FRAGS_SIG = (int)(SEND_MSGSIZE_SIG/MTU)
if SEND_FRAGS_SIG == 0:
    SEND_FRAGS_SIG = 1
SEND_FRAGS_REF = (int)(SEND_MSGSIZE_REF/MTU)
if SEND_FRAGS_REF == 0:
    SEND_FRAGS_REF = 1

RECV_MSGSIZE_CFAR = N*ROWS/8   # bit matrix
RECV_MSGSIZE_AF = N*ROWS*2
    
RECV_FRAGS_CFAR = (int)((RECV_MSGSIZE_CFAR)/MTU)
if RECV_FRAGS_CFAR == 0:
    RECV_FRAGS_CFAR = 1
RECV_FRAGS_AF = (int)((RECV_MSGSIZE_AF)/MTU)
if RECV_FRAGS_AF == 0:
    RECV_FRAGS_AF = 1

RECV_MSGSIZE = RECV_MSGSIZE_CFAR + RECV_MSGSIZE_AF
RECV_FRAGS = RECV_FRAGS_CFAR + RECV_FRAGS_AF


# ============================================================
# Оформляем тестовую последовательность во фрейм и блок данных
# ============================================================


# >i4 32bit int, big endian
# <i4 32bit int, little endian

# структура фрейма: четыре unsigned int32, по краям магические числа
# 575757 + #Op + CRC(msg) + 919191
# #Op: =0 for FFT, =1 for IFFT
frame = np.empty(4, dtype='<u4')
frame[0] = np.uint32(575757)
frame[1] = np.uint32(9)
frame[2] = np.uint32(0)
frame[3] = np.uint32(919191)
framebyte = np.ndarray(shape=FRAMESIZE, dtype='u1', buffer=frame)
#vhex = np.vectorize(hex)
#print('framebyte:', vhex(framebyte))

# ====================================================
# Генерим тестовую последовательность из трех сигналов
# ====================================================

# FIXME comment for text
t = np.linspace(0, N+ROWS, N+ROWS)
#s = (N+ROWS-t)*K*np.exp(1j * 2.0*np.pi*t/(N+ROWS))
s = K*np.exp(1j * 2.0*np.pi*t/(N+ROWS))
sig = s
ref = s

msg_sig = np.empty((N+ROWS) * 2, dtype='<i2')
msg_ref = np.empty(N * 2, dtype='<i2')

# ====================================================
# XXX порядок следования - ImRe !!!
msg_sig[0::2] = np.int16(sig.imag)
msg_sig[1::2] = np.int16(sig.real)
msg_ref[0::2] = np.int16(ref[:N].imag)
msg_ref[1::2] = np.int16(ref[:N].real)

send_msgbyte_sig = np.ndarray(shape=SEND_MSGSIZE_SIG, dtype='u1', buffer=msg_sig)
send_msgbyte_ref = np.ndarray(shape=SEND_MSGSIZE_REF, dtype='u1', buffer=msg_ref)

# ============================================================


# ==========================
# Вызываем DSP сервер по UDP
# данные посылаются и принимаются частями
# для обеспечения < MTU
# ==========================

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# ========== 1-я посылка: TARGET ===============
# посылаем заголовок
client_socket.sendto(framebyte, (SERVER, PORT))
print('sent frame:', len(framebyte))

x = int(SEND_MSGSIZE_SIG/SEND_FRAGS_SIG)
for i in range(SEND_FRAGS_SIG):
    # посылаем массив для FFT частями
    n = client_socket.sendto(send_msgbyte_sig[i*x:(i+1)*x], (SERVER, PORT))
    #print('#', i+1, n, 'sent msg from', i*x, 'to', (i+1)*x)
    sleep(0.01) # Time in seconds.
print('Sent SIG', SEND_FRAGS_SIG, 'frags', x, 'bytes')

# ========== 2-я посылка: REF ===============
# посылаем заголовок
client_socket.sendto(framebyte, (SERVER, PORT))
print('sent frame:', len(framebyte))

x = int(SEND_MSGSIZE_REF/SEND_FRAGS_REF)
for i in range(SEND_FRAGS_REF):
    # посылаем массив для FFT частями
    n = client_socket.sendto(send_msgbyte_ref[i*x:(i+1)*x], (SERVER, PORT))
    #print('#', i+1, n, 'sent msg from', i*x, 'to', (i+1)*x)
    sleep(0.01) # Time in seconds.
print('Sent REF', SEND_FRAGS_REF, 'frags', x, 'bytes')
    
# ============ принимаем FFT частями =========================

# ============================================================
# Принимаем вместе побайтно CFAR + AF
# ============================================================

print('Waiting CFAR + AF', RECV_MSGSIZE_CFAR, '+', RECV_MSGSIZE_AF, '=', RECV_MSGSIZE, 'bytes', 'num frags:', RECV_FRAGS)

x = int(RECV_MSGSIZE_CFAR/RECV_FRAGS_CFAR)
recv_msgbyte_cfar = np.empty(shape=RECV_MSGSIZE_CFAR, dtype='u1')
for i in range(RECV_FRAGS_CFAR):
    # принимаем массив для FFT частями
    data = client_socket.recv(x)
    #print(x, len(data))
    bdata = np.ndarray(shape=x, dtype='u1', buffer=data)
    recv_msgbyte_cfar[i*x:(i+1)*x] = bdata
    #print('\t#', i+1, 'recv msg from', i*x, 'to', (i+1)*x)

x = int(RECV_MSGSIZE_AF/RECV_FRAGS_AF)
recv_msgbyte_af = np.empty(shape=RECV_MSGSIZE_AF, dtype='u1')
for i in range(RECV_FRAGS_AF):
    # принимаем массив для FFT частями
    data = client_socket.recv(x)
    #print(x, len(data))
    bdata = np.ndarray(shape=x, dtype='u1', buffer=data)
    recv_msgbyte_af[i*x:(i+1)*x] = bdata
    #print('\t#', i+1, 'recv msg from', i*x, 'to', (i+1)*x)

client_socket.close()

fdata_cfar = np.empty(N*ROWS, dtype=float)
fdata_af = np.empty(N*ROWS, dtype=float)

# преобразование битового массива во float массив
data8 = np.ndarray(shape=N*ROWS/8, dtype='u1', buffer=recv_msgbyte_cfar)
fdata_cfar = np.unpackbits(data8, axis=0)
rdata_cfar = fdata_cfar.reshape(ROWS, N)
tdata_cfar = np.transpose(rdata_cfar)
print('CFAR to float:', rdata_cfar.size)

# ==========================
# преобразование af массива в float массив TODO тоже 8 бит а не 16!
data16 = np.ndarray(shape=N*ROWS, dtype='u2', buffer=recv_msgbyte_af)
fdata_af = data16
rdata_af = fdata_af.reshape(ROWS, N)
tdata_af = np.transpose(rdata_af)
#tdata_af = rdata_af
print('AF to float:', rdata_af.size)
print(rdata_af)
# ==========================

# ==========================
# Графика
# ==========================

'''
framet = N
framef = 256

fig = plt.figure(1)

ax1 = fig.add_subplot(311)
ax1.plot(s1[:framet].real, label='Input samples')
ax1.legend()
ax1.set_title('DSP output')

ax2 = fig.add_subplot(312)
ax2.plot(tdata_cfar.reshape(ROWS*N), label='TRANSPOSE CFAR')
ax2.legend()

ax3 = fig.add_subplot(313)
ax3.plot(tdata_af.reshape(ROWS*N), label='TRANSPOSE AF')
ax3.legend()
'''
#exit(0)
# 3D
print('Begin 3D show...')
fig2 = plt.figure(2)
tscale = np.linspace(0, ROWS, ROWS)
fscale = np.linspace(0, N, N)
ax = fig2.add_subplot(111, projection='3d')
X, Y = np.meshgrid(fscale, tscale)
ax.plot_surface(X, Y, abs(rdata_af), rstride=1, cstride=1, cmap=cm.coolwarm, linewidth=0, antialiased=False)
ax.set_xlabel('Dopler')
ax.set_ylabel('Delay')
ax.set_zlabel('AF')
plt.show()
