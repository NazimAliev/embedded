#!/usr/bin/python3

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


"""
A simple example of an animated plot
"""
import os,sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
sys.path.append('../../libs')
import libadapt as la

# генерация цели
# координаты x, y в последовательные моменты времени t
def xGen(N):
	PI = 2*np.pi/N
	y = np.zeros((N,2))
	t = np.arange(-N/2,N/2,1)
	y[:,0] = 10.0*t + 200.0
	y[:,1] = 1.0*t*t*t + 5000.0
	return y

def xGen2(N):
	y = np.zeros((N,2))
	for t in np.arange(N):
		f = (2 * np.pi * t) / N
		y[t,0] = 2.0 * np.sin(f)
		y[t,1] = np.cos(f)
	return y

# alpha beta фильтр
# y[N] является предсказанным значением
def abFilter(X):

	# константы
	dt = 1.0    # шаг по времени
	a = 0.95    # alpha
	b = 0.5 # beta

	# переменные
	xk_1 = 0 # предыдущее значение x
	vk_1 = 0.0 # предыдущее значение v
	xk = 0   # текущее значение x
	vk = 0.0   # текущее значение v
	rk = 0.0   # текущее значение ошибки

	N = X.size
	y = np.zeros(N, dtype=float)

	for t in range(N):
		#print(t)
		# получили прогноз, основанный на прошлом шаге
		# новые значения X, Y здесь не играют
		xk = xk_1 + ( vk_1 * dt )
		vk = vk_1
		#print("x:", X[t].astype(int), "xk:", xk)

		y[t] = xk

		# оцениваем точность сделанного прогноза, основываясь на поступивших реальных значениях
		rk = X[t] - xk
		#print("delta:", rk, "v:", vk_1)
		#print("--")

		# делаем прогноз на следующий шаг - предсказание
		xk = xk + a * rk
		vk = vk + ( b * rk ) / dt

		# предсказанные значения сохраняются и будут использованы на следующем шаге
		xk_1 = xk
		vk_1 = vk

	return y


'''
 BEGIN PROGRAM

 TODO:
 разложить входной сигнал на ортогональные компоненты
 пропустить ортогональные компоненты через альфа-бета фильтр
 восстановить из компонент сигнал
'''

N =  32

x = xGen(N)
ax = abFilter(x[:,0])
ay = abFilter(x[:,1])

#N = 2

#x = np.zeros((N,2))
#x[0,:] = [4,2]
#x[1,:] = [3,2.5]

p = np.zeros((N,2))
v = np.zeros((N,2))

for i in range(N-1):
	p[i+1,:] = x[i,:]*(np.inner(x[i+1,:],x[i,:])/np.inner(x[i,:],x[i,:]))
	v[i+1,:] = x[i+1,:] - p[i+1,:]
	
hv = np.hypot(v[:,0],v[:,1])
hp = np.hypot(p[:,0],p[:,1])
vv = abFilter(hv)
pp = abFilter(hp)

#vv=hv
#pp=hp
k_v = vv/hv
k_p = pp/hp

xx = p[:,0]/k_p + v[:,0]/k_v
yy = p[:,1]/k_p + v[:,1]/k_v

plt.figure(1)
plt.plot(x[:,0], x[:,1], 'go', label='Input')
plt.plot(ax, ay, 'ro', label='Plain AB filter')
plt.plot(xx, yy, 'r^', label='Orthogonal')
plt.legend()
#plt.figure(2)
#plt.plot(ax_cum, ay_cum, 'ro', label='Предсказанные')
#plt.plot(q[:,1], 'ro', label='Ошибка')
#plt.plot(ye, 'g^', label='Ошибка')
plt.grid(True)
plt.xlabel('X')
plt.ylabel('Y')
#plt.legend()
plt.title("Траекторная обработка Alpha-Beta фильтром\n\nNazim.RU")
plt.show()
