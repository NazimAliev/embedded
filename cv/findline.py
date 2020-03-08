#!/usr/bin/python3
# -*- coding: utf-8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

import numpy as np
import matplotlib.pyplot as plt

# rotate matrix
def rotate(X, rot):
	rz = np.pi*rot/360.0
	cc = np.cos(rz)
	ss = -np.sin(rz)
	M = np.mat([[cc, -ss],[ss, cc]])
	Y = M*X
	return Y

a = np.mat([3,5]).T
b = rotate(a, 20)
print(b)
c = rotate(b,-20)
print(c)
''' two lines imitator '''
SIZE = 32 
x1 = np.linspace(0,16,num=6)
y1 = 0.5*x1 + 1.2
x2 = np.linspace(0,16,num=10)
y2 = -1.2*x2 + 25.2
x = np.append(x1,x2)
y = np.append(y1,y2)
# z contains x,y points of two lines
z = np.append([x],[y],axis=0)
Z = np.mat(z)

shift = 15*4

''' rotate scene to find vertical lines after rotation '''
# array contains [angle, max bincount]
bc = np.array([[],[],[]])
for rot in range(0,360,5):
	# rotate two lines to 10 deg step angle
	MZ = rotate(Z, rot)
	# only positive values for bincount - shif x-axis (must be compensate later!)
	xz = np.around(MZ[0,:]) + shift
	# what x-axis point contains max point of rotated lines
	binc = np.bincount(np.ravel(xz[0]).astype(int))
	# update array
	bc = np.append(bc, [[rot],[np.argmax(binc)],[np.amax(binc)]],axis=1)
#print(bc)

# index of good x-axis points
idx = np.array(np.where(bc[2,:] > 5.0)[0])
lines = idx.size
res = bc[:,idx]
print(res)
#print(res[0])

plt.figure(0)
''' restore lines '''
# res[0,:] - rotated angles
# res[1,:] - vertical line shift on x-axis (after rotated)
# res[2,:] - bincount (already used, no need in future) 
for i in range(lines):
	angle = -res[0,i]
	print("[%d] line angle: %f" % (i, angle))
	# compensate shift and align line
	p1 = [res[1,i] - shift,-20]
	p2 = [res[1,i] - shift,30]
	print(p1,p2)
	p1rot = np.ravel(rotate(np.mat(p1).T, angle))
	p2rot = np.ravel(rotate(np.mat(p2).T, angle))
	print(p1rot,p2rot)
	# plt.plot([x1,x2,x3],[y1,y2,y3])
	plt.plot([p1[0],p2[0]], [p1[1],p2[1]])
	plt.plot([p1rot[0],p2rot[0]], [p1rot[1],p2rot[1]])
# 
plt.plot(Z[0,:],Z[1,:],'-bo')
MZ = rotate(Z,80) 
plt.plot(MZ[0,:],MZ[1,:],'-go')
MZ = rotate(Z,230) 
plt.plot(MZ[0,:],MZ[1,:],'-yo')
plt.show()
