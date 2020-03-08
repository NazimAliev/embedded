#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


import numpy as np
np.set_printoptions(precision=2)

x = np.array([[1,2,3],[4,5,6],[7,8,9]])
L, ev = np.linalg.eig(x)

print("Матрица трансформации x:")
print(x)
print("")

print("Собственные векторы матрицы ev:")
print(ev)
print("")

print("Собственные числа матрицы L:")
print(L)
print("")

print("Проверка:")
print("")
print("x * ev:")
print(np.mat(x)*ev)
print("")
print("L * ev:")
print(L*ev)

print("ev[0,:] * x:")
print(ev[0,:]*np.mat(x))
print("")
print("L * ev[0,:]:")
print(L*ev[0,:])

'''
f = np.linspace(-0.5, 0.5, len(psd))

plt.plot(f, 10 * np.log10(psd/max(psd)), label='User defined')
psd, ev = np.linalg.eig(np.diag((1,2,3)))
plt.plot(f, 10 * np.log10(psd/max(psd)), label='threshold method (100)')
psd, ev = np.linalg.eig(np.diag((1, 2, 3)))
plt.plot(f, 10 * np.log10(psd/max(psd)), label='AIC method (8)')
plt.xlabel('(c) BeerWare Corp')
plt.legend()
plt.axis([-0.5, 0.5, -120, 0])
plt.show()
'''
