#!/usr/bin/python
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

'''
testeigen.py
'''

import os,sys
import numpy as np
import matplotlib.pyplot as plt

np.set_printoptions(precision=3)

# количество выборок сигнала
N = 100 

x = np.zeros((2,N))
x[0,:] = np.random.normal(0,1,N)
x[1,:] = np.random.normal(0,1,N)
x = np.mat(x)
COVN = np.cov(x)
print("\nCOV(x)\n")
print(COVN)

# матрица вращения
theta = np.pi*20.0/180.0
ROT = np.mat([[np.cos(theta), -np.sin(theta)],
	[np.sin(theta), np.cos(theta)]])

print("\nROT\n")
print(ROT)
# матрица масштабирования
SCALE = np.mat([[4.0,0],[0,0.5]])
print("\nSCALE\n")
print(SCALE*SCALE)

print("\nA=ROT*SCALE\n")
A = ROT*SCALE
print(A)
print("\nA*A.T\n")
print(A*A.T)

xa = A*x
COV = np.cov(xa)
print("\nCOV(A*x)\n")
print(COV)

'''
t = np.arange(N)
s1 = np.sin(1.5*np.pi*t/N)
s2 = np.sin(1.8*np.pi*t/N)
s = s1 + s2
print("\nCOVA(s)\n")
print(np.cov(s))
'''

# собственный вектор
[en,ev]=np.linalg.eig(COV)
#ev /= en 
print("\nEigenVector(COV)\n")
print(ev)
print("\nEigenValies(COV)\n")
print(en)

#print(X[0,:])
#print(X[1,:])

#XB = np.mat(ev).T*XA
SCALEA = np.sqrt(en)
#AA = ROTA*SCALEA
AA = np.mat(ev.T)

xb = AA*xa
# convert matrix to array - if not then plot show error data!

X = np.squeeze(np.asarray(x))
XA = np.squeeze(np.asarray(xa))
XB = np.squeeze(np.asarray(xb))

plt.figure(1)
plt.axes().set_aspect('equal', 'datalim')
plt.scatter(XA[0,:],XA[1,:], color='g', marker='.', label="Rotated and Scaled")
plt.scatter(XB[0,:],XB[1,:], color='r', marker='.', label="Rotated and Scaled")
plt.xlabel('(c) Nazim.RU')
plt.title('Orto')
plt.legend()
plt.grid(True)

plt.figure(2)
plt.axes().set_aspect('equal', 'datalim')
plt.scatter(X[0,:],X[1,:], color='b', marker='.',  label="Normal distr")
plt.scatter(XA[0,:],XA[1,:], color='g', marker='.', label="Rotated and Scaled")
plt.arrow(0, 0, en[0]*ev[0,0], en[0]*ev[1,0], head_width=0.5, head_length=0.5, fc='r', ec='r')
plt.arrow(0, 0, en[1]*ev[0,1], en[1]*ev[1,1], head_width=0.5, head_length=0.5, fc='r', ec='r')
plt.xlabel('(c) Nazim.RU')
plt.title('Eigenvectors')
plt.legend()
plt.grid(True)

plt.show()
