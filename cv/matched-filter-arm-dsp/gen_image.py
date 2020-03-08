#!/usr/bin/python3
# -*- coding: utf8 -*-

#
# Copyright (c) by Nazim Aliev
# All rights reserved.
#
# nazim.ru@gmail.com
#

# gen_image.py
'''
Check VBLS ARM/DSP project data

2d image filtration based on spiral template 

scene: input signal, contains useful signal ref and background picture (noise)
ref: known reference signal, use as reference
filter: output filter

data processing follows convolve in frequency domain: matched filter

SIGNAL = FFF(scene)*FFT(ref).conj
a SIGNAL peak placement correspondes to place the ref on scene
'''
 
# import the necessary packages
import cv2
import os,sys
import numpy as np
from matplotlib import pyplot as plt

# short log complex img data
def logArray(title, buf, N):
	print('%s ====================' % title)
	for i in range(N):
		if(i>2 and i<N-2):
			continue
		for j in range(N):
			if(j>2 and j<N-2):
				continue
			print("[%03d,%03d] Re %d\t" % (i,j,buf[i,j].real),end='')
			print("Im %d" % (buf[i,j].imag))

# function to test dsp fft2 - compare internal data	
def userFft2(buf, N):
	row=np.zeros((N,N),dtype=complex)
	fft=np.zeros((N,N),dtype=complex)
	tmp=np.zeros((N,N),dtype=complex)
	for i in range(N):
		row[i,:] = np.fft.fft(buf[i,:])	
		tmp[i,:] = row[:,i]	
	for i in range(N):
		fft[:,i] = np.fft.fft(row[:,i])	
	logArray('fft',(16448*fft)/65792, N)
	return fft

def norm(buf):
	res = buf;
	res /= (np.amax(np.abs(buf)))/32767
	return res
'''
read data from files or generate test sequence
'''

if 1:
	# read real img data from files
	PATH = '/home/legolas/Dropbox/dev/pics/cv.utils/'
	# get reference picture
	unit0 = cv2.imread(PATH+'spiral2.jpg', cv2.IMREAD_GRAYSCALE)
	# different scales for unit and template to test matching with different unit size
	scale_unit = 0.4
	scale_template = 0.4 
	# use unit to put on scene
	unit = cv2.resize(unit0,None,fx=scale_unit, fy=scale_unit, interpolation = cv2.INTER_CUBIC)
	# use template for matching
	template = cv2.resize(unit0,None,fx=scale_template, fy=scale_template, interpolation = cv2.INTER_CUBIC)

	#get scene picture
	#scene = cv2.imread('/home/legolas/Dropbox/Prima/nature.jpg', cv2.IMREAD_GRAYSCALE)
	scene = cv2.imread(PATH+'mahjong.jpg', cv2.IMREAD_GRAYSCALE)
	# ref template coordinates on scene
	sx,sy = (50,30)
	ux,uy = unit.shape
	# put reference block on scene
	scene[sx:sx+ux,sy:sy+uy] = unit 

	file_size = (512,512)
	sig = np.zeros(file_size,dtype=np.uint8)
	ref = np.zeros(file_size,dtype=np.uint8)

	xx,yy = scene.shape
	sig[:xx,:yy] = scene
	ref[:ux,:uy] = unit 
	
	# use only once: write img data for dsp in reduced square size NxN
	if 0:
		sig.tofile('img_sig.bin')
		ref.tofile('img_ref.bin')

	#sig = sig - np.mean(sig)
	#ref = ref - np.mean(ref)

else:
	# generate test sequence
	N=512
	sig=np.zeros((N,N),dtype=complex)
	ref=np.zeros((N,N),dtype=complex)
	for i in range(N):
		for j in range(N):
			sig[i,j] = 2*N*i+2*j + 1j*(2*N*i+2*j+1) 
			ref[i,j] = 2*N*i+2*j+2 + 1j*(2*N*i+2*j+3)

#sig=np.transpose(sig)
#ref=np.transpose(ref)
if 1:
	# ======== filtration ========
	SIG = np.fft.fft2(sig)
	REF = np.fft.fft2(ref)
	REF_CONJ = np.conj(REF)
	SIG = norm(SIG)
	REF_CONJ = norm(REF_CONJ) 
	OUT = SIG*REF_CONJ
	OUT = norm(OUT) 
	# ============================

	out = np.fft.ifft2(OUT)
	out = norm(out) 
else:
	# use dsp img output instead of input data generated here
	N=512
	tmp=np.zeros(N*N,dtype=complex)
	dsp = np.fromfile('img_out.bin', dtype=np.int16)
	print(dsp.shape)
	tmp.real=dsp[0::2]
	tmp.imag=dsp[1::2]
	out = tmp.reshape(N,N)

logArray('fft(ref).conj',(14424*REF_CONJ)/32767,512)
logArray('fft(sig)',(22394*SIG/32767),512)

# normalization for plotting
#out_abs = np.abs(np.fft.ifftshift(out))
out_abs = np.abs(out)

out_m = np.amax(out_abs)
argmax = np.argmax(out_abs)
argmax2d = np.unravel_index(argmax, np.array(out_abs).shape)
print(argmax2d)
print(out_m)
x,y = argmax2d
exit(0)
# drawing
plt.subplot(121)
scene_i = cv2.cvtColor(scene,cv2.COLOR_GRAY2RGB)
cv2.rectangle(scene_i,(y, x), (y+uy,x+ux), (10,255,10), 2)
plt.imshow(scene_i,cmap = 'gray')
plt.title(u'Сигнал'), plt.xticks([]), plt.yticks([])

plt.subplot(122)
out_log = np.log10(out_abs/out_m + 1.0)
plt.imshow(out_log,cmap='gray')
plt.title(u'Согласованный фильтр'), plt.xticks([]), plt.yticks([])

plt.show()
