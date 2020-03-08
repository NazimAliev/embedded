#!/usr/bin/python3

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

"""
A simple example of an animated plot
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# генерация цели
# координаты x, y в последовательные моменты времени t
def xGen(N):
	x = np.zeros(N, dtype=float)
	y = np.zeros(N, dtype=float)
	for t in range(N):
		f = (2 * np.pi * t) / N
		x[t] = 2.0 * np.sin(f) 
		y[t] = np.cos(f)
	return x, y

def xGen2(N):
	x = np.zeros(N, dtype=float)
	y = np.zeros(N, dtype=float)
	for t in np.arange(N):
		if t < N/2:
			x[t] = t
			y[t] = 2*t
		else:
			x[t] = 2*t - N/2 
			y[t] = t + N/2 
	return x, y

# alpha beta фильтр
# y[N] является предсказанным значением
def abFilter(x):
	dt = 0.5	# шаг по времени
	xk_1 = 0.0	# предыдущее значение x
	vk_1 = 0.0	# предыдущее значение v
	a = 0.85	# alpha
	b = 0.1	# beta

	xk = 0.0	# текущее значение x
	vk = 0.0	# текущее значение v
	rk = 0.0	# текущее значение ошибки

	N = x.size
	y = np.zeros(N, dtype=float)
	yf = np.zeros(N, dtype=float)

	for t in range(N):
		xk = xk_1 + ( vk_1 * dt )
		vk = vk_1

		rk = x[t] - xk

		xk = xk + a * rk
		vk = vk + ( b * rk ) / dt

		xk_1 = xk
		vk_1 = vk

		y[t] = xk
		yf[t] = xk + (vk * dt)
		print(t, "x:", x[t], "rk:", np.around(rk, 1), "yf:", yf[t])

	return y, yf

'''
 BEGING PROGRAM
'''

N = 40

xg, yg = xGen(N)
xa, xf = abFilter(xg)
print (xg)
print (xa)
ya, yf = abFilter(yg)

plt.plot(xg, yg, 'bo')
#plt.plot(xa, ya, 'r+')
plt.plot(xf, yf, 'ro')
plt.grid(True)

plt.legend(['Input targets', 'Tracked output'])
plt.xlabel('x data')
plt.ylabel('y data')
plt.show()
