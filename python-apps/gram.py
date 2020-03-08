#!/usr/bin/python3

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

"""
Последовательное решение задачи оргогонализации
"""
import numpy as np
import matplotlib.pyplot as plt
import datetime as dt
np.set_printoptions(precision=1)

'''
 BEGING PROGRAM
Rxx * Wopt = Pdx
Wopt = R−1xx * Pdx
'''

rxx = np.zeros((2, 2))
pdx = np.zeros((2, 1))

x1 = np.array([-15, 35, 50, 50, 25])
x2 = np.array([-20, 30, 40, 30, -5])
d = np.array([-10, 10, 0, 10, -10])

print("x1:", x1)
print("x2:", x2)
print("d:", d)

rxx[0, 0] = np.dot(x1, x1)
rxx[0, 1] = np.dot(x1, x2)
rxx[1, 0] = np.dot(x2, x1)
rxx[1, 1] = np.dot(x2, x2)
print("rxx:", rxx)

pdx[0] = np.dot(x1, d)
pdx[1] = np.dot(x2, d)
print("pdx:", pdx)

print("inv:", np.linalg.inv(rxx) * pdx)
w = np.linalg.solve(rxx, pdx)
print("w:", w)
dest = w[0]*x1 + w[1]*x2 
print("dest:", dest)
print("xest:", dest - d)

proj_x2_to_x1 = x1 * (np.inner(x2, x1)/np.inner(x1, x1))
print("proj_x2_to_x1:", proj_x2_to_x1)
v1 = x1
v2 = x2 - proj_x2_to_x1
print("v2:", v2)

print(np.inner(v1, v2))

proj_d_to_v1 = v1 * (np.inner(d, v1)/np.inner(v1, v1))
print("proj_d_to_v1:", proj_d_to_v1)
proj_d_to_v2 = v2 * (np.inner(d, v2)/np.inner(v2, v2))
print("proj_d_to_v2:", proj_d_to_v2)
print("dest based on vX:", proj_d_to_v1 + proj_d_to_v2)

'''
1000 0:00:00.119958
2000 0:00:00.816943
4000 0:00:05.747642
6000 0:00:18.522508
8000 0:00:43.691010

M = 8000
a = 5 * np.random.random_sample((M, M)) - 5
b = 5 * np.random.random_sample((M, 1)) - 5
n1=dt.datetime.now()
z = np.linalg.solve(a, b)
n2=dt.datetime.now()
print(M, n2-n1)
'''

exit(0)
pdx = np.correlate(d, x, "full")[L-1:]

N = 20

xg, yg = xGen(N)
xa = abFilter(xg)
ya = abFilter(yg)

plt.plot(xg.astype(int), yg.astype(int), 'go')
plt.plot(xa.astype(int), ya.astype(int), 'ro')
plt.grid(True)

plt.xlabel('Time')
plt.ylabel('Target distance')
plt.show()
