#!/usr/bin/python
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

'''
music.py
'''

import os,sys
import numpy as np
import matplotlib.pyplot as plt

np.set_printoptions(precision=2)

# количество выборок сигнала
N = 200 
# количество элементов АР
M = 10
# длина волны
lwave = 150
# расстояние между элементами АР
d = lwave / 2
snr = 20
# количество сигналов с разными частотами
P = 4

# вектор: сдвиги фаз с линейной АР в зависимости от угла прихода сигнала theta
# поскольку сигналы в узкой полосе, то их влияние на величину фазы не учитывается 
def getAperture(theta):
	m = np.arange(M)
	phases = np.exp(-1j*2.0*np.pi*(lwave/2)*m*np.sin(theta)/lwave)
	return phases 

# частота выборок
w = np.mat([np.pi/4, np.pi/3, np.pi/5, np.pi/6]).H
assert(w.size == P)
nn = np.mat(np.arange(1,N+1))
wn = w*nn
# матрица сигнала (P,N)
# каждая строка соответствует одной частоте
# значение в строке - выборка синусоиды
x=2.0*np.exp(1j*(wn))
x += np.random.normal(0,100,N)

# направление прихода сигналов разных частот
doa = np.array([40.0, 60.0, 70.0, -10.0]) / 180.0*np.pi
assert(doa.size == P)

# матрица фазовращателей на элементах решетки
# фазовый раскрыв
aperture = np.zeros((P, M), dtype=complex)

for k in range(P):
	aperture[k,:] = getAperture(doa[k])

APERTURE = np.mat(aperture).H
# формирование сигнала во времени
# умножение на фазовращатели
# результат матрица (M,N)
# P ушло потому что сигналы разных частот на каждом диполе
# умножились на фазовый сдвиг и просуммировались
ax=APERTURE*x

# корр. функция сигнала
# (M,M)
# Rxx = np.mat(sla.toeplitz(xsum))
Rxx = ax*ax.H
print("\nCovariation matrix Rxx\n")
print(Rxx.shape)

'''
Rxx является hermitian matrix, поскольку
Rxx == Rxx.H и тогда
eigenvectors are mutually orthogonal
(the dot-product between any two eigenvectors is zero)
and the eigenvalues are real
'''

# собственный вектор
[en,ev]=np.linalg.eig(Rxx)
# расположить в порядке убывания
idx = en.argsort()[::-1] 
en = en[idx] 
ev = ev[:,idx] 
ev /= en 
print("\nEigen vectors\n")
print(ev)
print("\nEigen valies\n")
print(en)
EV = np.mat(ev[:,0:(M-P)])

'''
Матрица Rxx разлагается по eigenvectors
Rxx == np.mat(ev)*np.mat(np.diag(en)).T*np.mat(ev).T

проверка - сигнальное и шумовое subspace ортогональны
EVSignal = np.mat(ev[:,:EP])
print(EV*EVSignal)
'''

theta = np.arange(-90,90+0.5,0.5)/180.0*np.pi
Pmusic = np.zeros(theta.size)

s=np.zeros(M, dtype=complex)
for i in range(theta.size):
	a = getAperture(theta[i])
	A = np.mat(a)
	P=A*EV*EV.H*A.H
	Pmusic[i]=np.absolute(1/P)

Pmusic = 10*np.log10(Pmusic/np.max(Pmusic))
plt.figure(0)
plt.plot(180*theta/np.pi, Pmusic, '-k')
plt.xlabel('angle theta/degree')
plt.ylabel('spectrum function P(theta) /dB')
plt.title('DOA estimation based on MUSIC algorithm ')
plt.grid(True)
plt.show()
