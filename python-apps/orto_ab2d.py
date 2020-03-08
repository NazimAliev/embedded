#!/usr/bin/python3

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def xyGen():
	# --->
	z = np.arange(start=1, stop=11, step=1) + 1j * np.zeros(10)

	# )
	t = np.linspace(-np.pi/2, np.pi/2, 12)
	tmp = 2.5*np.cos(t) + 11.0 + 1j * (-2.5*np.sin(t) - 2.5)
	z = np.append(z, tmp)

	# <---
	tmp = np.arange(start=10, stop=0, step=-1) + 1j * (np.zeros(10) - 5)
	z = np.append(z, tmp)

	# (
	t = np.linspace(np.pi/2, -np.pi/2, 12)
	tmp = -2.5*np.cos(t) + 1j * (2.5*np.sin(t) - 7.5)
	z = np.append(z, tmp)

	# --->
	tmp = np.arange(start=1, stop=11, step=1) + 1j * (np.zeros(10) - 10.0)
	z = np.append(z, tmp)

	rot = 0.4 
	zrot = z * np.exp(1j * rot)

	#return x, y
	return zrot

# вектор в комплексное число
def a2c(x):
	return x[0] + 1j*x[1]

# комплексное число в вектор
def c2a(x):
	return np.array([x.real, x.imag])

# разложение каждого вектора трека в базисе предыдущего вектора
def ortoF(Z):
	N = Z.size
	# пред-предыдущий вектор
	z2 = Z[0]
	# предыдущий вектор
	z1 = Z[1]
	x0 = 0

	out = np.zeros(N, dtype='complex')

	for t in range(2,N):
		# текущий и предыдущий относительно начала координат
		z0 = Z[t] - z1
		z01 = z1 - z2
		az0 = c2a(z0)
		az01 = c2a(z01)

		# proj это массив, не комплексное число
		proj_az0_to_az01 = az01 * (np.inner(az0, az01)/np.inner(az01, az01))
		#print("proj_x2_to_x1:", proj_x2_to_x1)
		# пара x,v является разложением вектора z0 в базисе предыдущего вектора z01
		# x смотрит в сторону движения, v - перпендикулярно и является отклонением
		x = a2c(proj_az0_to_az01)	
		v = z0 - x
		# x,v - комплексные числа
		z2 = z1
		z1 = Z[t]
	return out
		
# alpha beta фильтр
# y[N] является предсказанным значением
def abFilterX(X):

	# константы
	dt = 1.0	# шаг по времени
	a = 0.95	# alpha
	b = 0.5 # beta

	# переменные
	x1 = 0.0	# предыдущее значение x
	v1 = 0.0	# предыдущее значение v
	x = 0.0	# текущее значение x
	v = 0.0	# текущее значение v
	r = 0.0	# текущее значение ошибки

	N = X.size
	y = np.zeros(N, dtype=float)

	for t in range(N):
		# получили прогноз, основанный на прошлом шаге
		# новые значения X, Y здесь не играют
		x = x1 + ( v1 * dt )
		v = v1

		y[t] = x

		# оцениваем точность сделанного прогноза, основываясь на поступивших реальных значениях
		r = X[t] - x

		# делаем прогноз на следующий шаг - предсказание
		x = x + a * r
		v = v + ( b * r ) / dt

		# предсказанные значения сохраняются и будут использованы на следующем шаге
		x1 = x
		v1 = v

	return y


# alpha beta фильтр
# z является предсказанным значением
def abFilter(Z):
	dt = 0.5	# шаг по времени

	# предыдущий вектор
	z1 = 0.0 + 1j * 0.0
	# текущий вектор 
	z = 0.0 + 1j * 0.0
	# предыдущая скорость 
	v1 = 0.0 + 1j * 0.0
	# текущая скорость 
	v = 0.0 + 1j * 0.0
	# ошибка
	r = 0.0 + 1j * 0.0

	a = 0.85	# alpha
	b = 0.2	# beta


	N = Z.size
	out = np.zeros(N, dtype='complex')

	for t in range(N):

		'''
		proj_x2_to_x1 = x1 * (np.inner(x2, x1)/np.inner(x1, x1))
		print("proj_x2_to_x1:", proj_x2_to_x1)
		v1 = x1
		v2 = x2 - proj_x2_to_x1
		'''

		# прогноз текущего положения вектора по предыдущему и скорости
		z = z1 + ( v1 * dt )
		v = v1
		#out[t] = z

		# оцениваем ошибку прогноза по известному вектору z[t] 
		r = Z[t] - z

		# корректировка прогноза, будет учтено в следующей итерации
		z = z + a * r
		v = v + ( b * r ) / dt

		z1 = z
		v1 = v

		#y[t] = zk
		#yf[t] = zk + (vk * dt)
		#print(t, "x:", x[t], "rk:", np.around(rk, 1), "yf:", yf[t])

	return out

'''
 BEGING PROGRAM
'''

z_in = xyGen()
#z = abFilter(z_in)
z = abFilterX(z_in.real) + 1j*abFilterX(z_in.imag)
#orto = ortoF(z_in)

plt.plot(z_in.real, z_in.imag, 'bo')
plt.plot(z.real, z.imag, 'r^')
#plt.plot(orto.real, orto.imag, 'rs')
plt.grid(True)

#plt.legend(['Input targets', 'Tracked output'])
plt.xlabel('x data')
plt.ylabel('y data')
plt.show()
