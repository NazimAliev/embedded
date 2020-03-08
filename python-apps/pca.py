#!/usr/bin/python3
# -*- coding: utf8 -*-

# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


import numpy as np
import matplotlib.pyplot as plt
np.set_printoptions(precision=3)

# cov matrix 
track = np.array([ [2.4,0.7,2.9,2.2,3.0,2.7,1.6,1.1,1.6,0.9],
            [2.5,0.5,2.2,1.9,3.1,2.3,2,1,1.5,1.1] ])

print("track")
print(track)

# computing eigenvalues and eigenvectors of covariance matrix
A = track.T
M = (A-np.mean(A.T,axis=1)).T # subtract the mean (along columns)
[evalue,evector] = np.linalg.eig(np.cov(M)) # attention:not always sorted
score = np.dot(evector.T,M) # projection of the track in the new space
print("cov")
print(np.cov(M))
print("cov(score)")
print(np.cov(score))

print("evector")
print(evector)
print(evector[:,0],evector[:,1])
print("evalue")
print(evalue)

plt.figure()
ax = plt.subplot(121)
# every eigenvector describe the direction
# of a principal component.

print("np.cov(M)")
print(np.cov(M))
plt.title('Трек в двумерной плоскости')
plt.xlabel('(c) Nazim.RU')

m = np.mean(track,axis=1)
#r0 = 2*evector.T*evalue
r0 = evector.T*evalue
r = r0.T
print("r")
print(r)
#plt.plot([0, -r[0,1]]+m[0], [0, r[0,0]]+m[1],'r')
#plt.plot([0, r[1,1]]+m[0], [0, -r[1,0]]+m[1],'r')
ax.set_xlim([0.4, 3.5])
ax.set_ylim([0.4, 3.5])
#ax.axis('equal')
ax.arrow(m[0], m[1], -r[0,1], r[0,0], head_width=0.1, head_length=0.2, fc='r', ec='r')
ax.arrow(m[0], m[1], r[1,1], -r[1,0], head_width=0.1, head_length=0.2, fc='r', ec='r')
ax.plot(track[0,:],track[1,:],'ob') # the track

plt.subplot(122)
# new track
print("np.cov(score)")
print(np.cov(score) - np.mean(score, axis=1))
plt.plot(score[0,:],score[1,:],'*g')
plt.title(u'Одномерный трек')
plt.axis('equal')
plt.show()
