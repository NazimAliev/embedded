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
	PI = 2*np.pi/N
	x = np.zeros(N, dtype=float)
	y = np.zeros(N, dtype=float)
	for t in range(N):
		tt = t - N/2
		x[t] = 100.0*tt
		y[t] = -(tt*tt*tt + 1000.0)
		#x[t] = 100.0*np.cos(PI*t)
		#y[t] = -100.0*np.sin(2*PI*t)
		#x[t] = 100.0*t
		#y[t] = -100.0*t
	return x, y

# alpha beta фильтр
# y[N] является предсказанным значением
def abFilter(X, Y):

	'''
	X,Y = значения по осям
	x,y = вход и выход
	'''
	# константы
	dt = 1.0	# шаг по времени
	a = 0.95	# alpha
	b = 0.5 # beta

	# переменные
	xk_1X = 0.0	# предыдущее значение x
	xk_1Y = 0.0	# предыдущее значение x
	vk_1X = 0.0	# предыдущее значение v
	vk_1Y = 0.0	# предыдущее значение v
	xkX = 0.0	# текущее значение x
	xkY = 0.0	# текущее значение x
	vkX = 0.0	# текущее значение v
	vkY = 0.0	# текущее значение v
	rkX = 0.0	# текущее значение ошибки
	rkY = 0.0	# текущее значение ошибки

	N = X.size
	yX = np.zeros(N, dtype=float)
	yY = np.zeros(N, dtype=float)

	for t in range(N):
		print(t)
		# получили прогноз, основанный на прошлом шаге
		# новые значения X, Y здесь не играют
		xkX = xk_1X + ( vk_1X * dt )
		xkY = xk_1Y + ( vk_1Y * dt )
		vkX = vk_1X
		vkY = vk_1Y
		print("x:", X[t].astype(int), "xkX:", xkX)
		print("y:", Y[t].astype(int), "xkY:", xkY)

		yX[t] = xkX
		yY[t] = xkY

		# оцениваем точность сделанного прогноза, основываясь на поступивших реальных значениях
		rkX = X[t] - xkX
		rkY = Y[t] - xkY
		print("delta:", rkX, "v:", vk_1X)
		print("--")

		# делаем прогноз на следующий шаг - предсказание
		xkX = xkX + a * rkX
		xkY = xkY + a * rkY
		vkX = vkX + ( b * rkX ) / dt
		vkY = vkY + ( b * rkY ) / dt

		# предсказанные значения сохраняются и будут использованы на следующем шаге
		xk_1X = xkX
		xk_1Y = xkY
		vk_1X = vkX
		vk_1Y = vkY

	return yX, yY

'''
 BEGING PROGRAM
'''

N = 32

xg, yg = xGen(N)
xa, ya = abFilter(xg, yg)

plt.plot(xg.astype(int), yg.astype(int), 'g^', label='Измеренные')
plt.plot(xa.astype(int), ya.astype(int), 'ro', label='Предсказанные')
plt.grid(True)

plt.xlabel('X')
plt.ylabel('Y')
plt.legend()
plt.title("Траекторная обработка Alpha-Beta фильтром\n\nNazim.RU")
plt.show()
