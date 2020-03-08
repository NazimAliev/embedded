#!/usr/bin/python3

# -*- coding: utf-8 -*-


# Copyright (c) by Nazim Aliev
# All rights reserved.

# nazim.ru@gmail.com


"""
Все функции монитора
"""

import os
import numpy as np
import socket as sc
import init_mon as im

# чтение тестового файла
def readv_iq(filename, count, offset):
	# prepare signal from file
	f = open(filename, 'rb')
	size = os.fstat(f.fileno()).st_size
	if(2 * (count + offset) > size):
		print("readv_iq error")
		return np.array([0])
	f.seek(2*offset)
	udata = bytearray(f.read(2*count))
	f.close
	data = np.array(udata, dtype=np.byte) - np.byte(127)

	y = data[0::2] + 1j * data[1::2]
	return y

# многопроцессный tcp сервер
# данные принимаются размером кратным NSAMPLES
def tcp_wrap(pmap):
	nt = pmap[0]
	tcp_len = pmap[1]
	#print("[%d] tcp wait %d b" % (nt, block_len))

	# с каких плат действительно придут данные - смотрим конфигурацию
	if(im.MODE == "zboard" and nt == 1):
		# в режиме zboard не дождемся данных с dsp (nt=1) - пропускаем
		return np.arange(tcp_len)
	if(im.MODE == "dsp" and nt == 0):
		# в режиме dsp не дождемся данных с zboard (nt=0) - пропускаем
		return np.arange(tcp_len)
	blocks = int(tcp_len / (2*im.NSAMPLES)) 
	#print("===", tcp_len, blocks)
	data = np.empty(0, dtype='i1')
	for i in range(blocks):
		bdata = tcp_recv(nt)
		data = np.append(data, bdata)
		#print("tcp_wrap: append %d bytes after tcp_recv" % (data.size))
	return data

# прием блока NSAMPLES
def tcp_recv(nt):
	client_socket, address = im.tcp[nt].accept()

	data = np.empty(0, dtype='i1')
	# блок состоит из NSAMPLES short
	remain_len = 2*im.NSAMPLES;

	while(1):
		udata = bytearray(client_socket.recv(remain_len))
		d = np.array(udata, dtype='i1')
		data = np.append(data, d)
		l = d.size
		if(l == 0):
			#print("Zero Data len")
			break;
		remain_len = remain_len - l
		#print("got:", l, "remain:", remain_len)
		if(remain_len == 0):
			break;
	client_socket.close()
	return data 

def rand(n):
	data = np.random.random(n)
	data[int(n*0.1):int(n*0.13)] += .5
	data[int(n*0.18)] += 2
	data[int(n*0.1):int(n*0.13)] *= 5
	data[int(n*0.18)] *= 20
	#data *= 1e-12
	return data, np.arange(n, n+len(data)) / float(n)

def showZboard(buf_zboard):
	x2 = np.arange(im.X/2)
	# каждый лог занимает в принятом буфере строку в NSAMPLES short 
	# в raw буфере это будет 2*NSAMPLES байт
	N = 2*im.NSAMPLES

	sig_i = np.frombuffer(buf_zboard[0:N], dtype='i4')
	ref_i = np.frombuffer(buf_zboard[N:2*N], dtype='i4')

	osig = np.frombuffer(buf_zboard[2*N:3*N], dtype='i2')
	osig_i = osig[0::2]
	print("sig_i[0:8]:", sig_i[0:8])
	print("osig_i[0:8]:", osig_i[0:8])
	print("")

	oref = np.frombuffer(buf_zboard[3*N:4*N], dtype='i2')
	oref_i = oref[0::2]
	print("ref_i[0:8]:", ref_i[0:8])
	print("oref_i[0:8]:", oref_i[0:8])
	print("")

	# левая панель
	im.p11.setData(x2, im.K1_SIG*sig_i[0:im.X/2])
	im.p12.setData(x2, im.K2_OSIG*osig_i[0:im.X/2])
	im.p13.setData(x2, im.K3_REF*ref_i[0:im.X/2])
	# правая панель
	im.p31.setData(x2, im.K3_REF*ref_i[0:im.X/2])
	im.p32.setData(x2, im.K4_OREF*oref_i[0:im.X/2])

'''
Правила для logMode расписаны в файле logrules.txt
'''

def showDsp(buf_dsp):
	#print("buf_dsp size:", buf_dsp.size)
	x2 = np.arange(im.X/2)
	# для центрирования спектра +-0
	xf1 = np.fft.fftfreq(im.X, 1.0/im.X)
	xf2 = np.fft.fftfreq(int(im.X/2), 2.0/im.X)
	xf4 = np.fft.fftfreq(int(im.X/4), 4.0/im.X)
	# каждый лог занимает в принятом буфере строку в NSAMPLES short 
	# в raw буфере это будет 2*NSAMPLES байт
	N = 2*im.NSAMPLES

	# 0) sig: input DSP
	s = np.frombuffer(buf_dsp[0:N], dtype='i2')
	#print("s size:", s.size)
	# порядок ImRe - так передаются данные sig, ref в afmath.c
	s_q = s[0::2]
	s_i = s[1::2]
	print("s_i[0:8]:", s_i[0:8])
	print("s_q[0:8]:", s_q[0:8])
	print("")

	# 1) conj(ref): input DSP
	r = np.frombuffer(buf_dsp[N:2*N], dtype='i2')
	# порядок ImRe - так передаются данные sig, ref в afmath.c
	r_q = r[0::2]
	r_i = r[1::2]
	print("r_i[0:8]:", r_i[0:8])
	print("r_q[0:8]:", r_q[0:8])
	print("")

	# 2) mul = short(sig * conj(ref)) (t<ROWS)
	mul = np.frombuffer(buf_dsp[2*N:3*N], dtype='i2')
	# порядок ImRe - результат работы complex mul в afmath.c
	mul_q = mul[0::2]
	mul_i = mul[1::2]
	print("mul_i[0:8]:", mul_i[0:8])
	print("mul_q[0:8]:", mul_q[0:8])
	print("")

	# 3) DSP32(mul) (t<ROWS)
	dsp = np.frombuffer(buf_dsp[3*N:4*N], dtype='i4')
	dsp_i = dsp[0::2]
	dsp_q = dsp[1::2]
	adsp = np.abs(dsp_i + 1j*dsp_q)
	print("max adsp[%d]=%.0f" % (np.argmax(adsp), np.amax(adsp)))
	print("")

	# 4) short(DSP) (t<ROWS)  
	sdsp = np.frombuffer(buf_dsp[4*N:5*N], dtype='i2')
	sdsp_i = sdsp[0::2]
	sdsp_q = sdsp[1::2]
	asdsp = np.abs(sdsp_i + 1j*sdsp_q)
	print("max asdsp[%d]=%.0f" % (np.argmax(asdsp), np.amax(asdsp)))
	print("")

	# 5) dot32(short(DSP)) (t<ROWS)
	dot = np.frombuffer(buf_dsp[5*N:6*N], dtype='u4')
	print("max dot[%d]=%.0f" % (np.argmax(dot), np.amax(dot)))
	print("")

	# 6) af16 (t<ROWS)
	amb = np.frombuffer(buf_dsp[6*N:7*N], dtype='u2')
	print("max amb[%d]=%.0f" % (np.argmax(amb), np.amax(amb)))
	print("")

	# левая панель
	im.p21.setData(x2, s_i[0:im.X/2])
	im.p22.setData(x2, r_i[0:im.X/2])
	im.p23.setData(x2, im.K2_MUL * mul_q[0:im.X/2])

	# правая панель
	im.p41.setData(xf4, im.K3_DSP * adsp[0:im.X/4])
	im.p42.setData(xf2, im.K4_SDSP * asdsp[0:im.X/2])
	im.p43.setData(xf2, im.K5_DOT * dot[0:im.X/2])
	im.p44.setData(xf1, im.K6_AMB * amb[0:im.X])

