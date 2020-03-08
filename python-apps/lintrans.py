#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com

import numpy as np
import matplotlib.pyplot as plt
np.set_printoptions(precision=2)

tst = np.mat([ [-3,-2,-2,-1,-1,0,0,1,1,2,2,3],
            [0,-1,2,-2,4,-3,6,-2,4,-1,2,0] ])
print(tst)

# поворот на 30 град
ang = -np.pi*30.0/180.0
A = np.mat([[np.cos(ang),-np.sin(ang)],[np.sin(ang),np.cos(ang)]])
# растяжение по горизонтали
A = np.mat([[2,0],[0,1]])
# сжатие по горизонтали
A = np.mat([[0.5,0],[0,1]])
# растяжение по вертикали 
A = np.mat([[1,0],[0,2]])
# перекос
A = np.mat([[1.2,0.5],[0.2,1.2]])

print("A")
print(A)

TST = A*tst

print("TST=A*tst")
print(TST)

[w,v] = np.linalg.eig(A) # attention:not always sorted

print("v")
print(v)
print("v1")
print(v[:,0])
print("v2")
print(v[:,1])
print("w")
print(w)

plt.figure()
#plt.subplot(121)
# every eigenvector describe the direction
# of a principal component.
plt.plot(tst[0,:],tst[1,:],'sb-')
plt.title(u'Преобразованный ромбик')
plt.xlabel('(c) Nazim.RU')

print("A*v1")
print(A*np.mat(v[:,0]))
print("w*v1")
print(w[0]*v[:,0])
print("A*v2")
print(A*np.mat(v[:,1]))
print("w*v2")
print(w[1]*v[:,1])

plt.plot(TST[0,:],TST[1,:],'or')
#plt.plot(r[0,0], r[1,0], 'Dg')
#plt.plot(r[0,1], r[1,1], 'Dg')

plt.axis('equal')
plt.show()
