#!/usr/bin/python3
# -*- coding: utf8 -*-

# sp_class.py
# robot states and actions
# food coordinates: x,y = 0,0
# robot coordinates: x=200, y=0, angle=0

import os,sys
import numpy as np

np.set_printoptions(precision=2)

''' ROBOT '''

# robot class
class Robot:
	# robot start position
	x = 200
	y = 0
	angle = np.pi/2
	sensor = 0

	# turn robot by left or right motor and move
	def motorL(self):
		self.angle += 5*np.pi/180
		if self.angle > np.pi:
			self.angle -= 2*np.pi
		self._move()
		#print("Motor L")

	def motorR(self):
		self.angle -= 5*np.pi/180
		if self.angle < -np.pi:
			self.angle += 2*np.pi
		self._move()
		#print("Motor R")
		
	def _move(self):
		self.x += 3*np.cos(self.angle)
		self.y += 3*np.sin(self.angle)
		print("angle: %.2f x: %.1f y: %.1f atn: %.2f" % (self.angle,self.x,self.y,np.arctan2(self.y,self.x)))
		#print(3*np.cos(self.angle),3*np.sin(self.angle))
		#self.direct = np.arctan2(self.y,self.x)

	# get gradient level from "food" point located at (0,0)
	def getSensor(self, t):
		#diff = np.arctan2(self.y,self.x) - self.angle
		diff = np.pi + np.arctan2(self.y,self.x) - self.angle
		if diff >= np.pi:
			diff -= 2*np.pi
		if diff <= -np.pi:
			diff += 2*np.pi
		#print("atn: %f.1 angle: %f.1 diff: %f.1" % (180*np.arctan2(self.y,self.x)/np.pi, 180*self.angle/np.pi, 180*diff/np.pi))
		return diff

	def getPos(self):
		return self.x,self.y,self.angle 

''' NEURON '''
	
# neuron class
class Neuron:

	def __init__(self, M, N, label):
		self.M = M 
		self.N = N 
		self.label = label 
		self.signal = np.zeros((M,N), dtype='int')
		self.last_spike_time = -N
		self.weight = np.ones(M)
		self.charge = 0
		self.charge_prev = 0
		# output
		self.WIN = 8
		self.FIRE_LEVEL = 1200 
		#self.FIRE_LEVEL = 500
		self.K_STDP = 1.5
		self.WMAX = 3.0
		# post STDP more strong than pre
		self.stdp_pre = np.exp(-np.arange(self.WIN)/1.0)
		self.stdp_post = 1.2*np.exp(-np.arange(self.WIN)/2.0)

	def update(self, s, t):
		self.signal[:,t] = s
		'''
		if s[0] == 1:
			print("[%d] S0" % t)
		if s[1] == 1:
			print("[%d] S1" % t)
		'''

		'''
		check POST spike. Neuron fire was in past, every t within window we define ONE inputs post spikes
		and penalty input with post spike. STDP rule requests max penalty for t+1 post spike
		and penalty decrease within window exp law
		'''

		# post spike delay after t is current time - spike time
		delay_post = t - self.last_spike_time
		if delay_post < self.WIN:
			#print("%s [%d] POST spike check %d times after spike" % (self.label, t, delay_post))
			# spike was not ling ago, because time inside window: check inputs
			for m in range(self.M):
				# check inputs after spike inside window 
				if(s[m] == 1):
					self.weight[m] = self.updateWeight(self.weight[m], delay_post, mode='post')
					#print("\t %s [-%d] w[%d]: %.1f" % (self.label, delay_post, m, self.weight[m]))

		sm = np.sum(self.signal[:,t] * self.weight)
		self.charge += sm*(1+0.05*self.charge) 
		if int(self.charge) != self.charge_prev:
			#print("%s [%d] Charge: %d" % (self.label, t, int(self.charge)))
			self.charge_prev = int(self.charge)

		'''
		update neuron acc and check PRE spike. Neuron fired right now, so recalculation ALL
		pre spikes window time size ago. STDP rule requests max premium for t pre spike
		and premium decrease within window exp law
		'''  
		if self.charge >= self.FIRE_LEVEL:
			# neuron fired
			spike = 1
			print("%s [%d] *** Spike! ***" % (self.label,t))
			self.charge = 0 
			self.last_spike_time = t
			# check all inputs before this spike inside window
			for i in range(self.WIN-1, 0, -1):
				delay_pre = self.last_spike_time - i + 1
				#print("%s [%d] PRE spike check %d times before spike" % (self.label, t, t-delay_pre))
				for m in range(self.M):
					if(self.signal[m,delay_pre] == 1.0):
						self.weight[m] = self.updateWeight(self.weight[m], i, mode='pre')
						#print("\t%s [+%d] w[%d]: %.1f" % (self.label, i, m, self.weight[m]))
		else:
			spike = 0

		return spike


	# update neuron input weight 'm' with STDP rule based on delay and mode=pre/post
	def updateWeight(self, w, delay, mode):
		if mode == "pre":
			# good input #m, increase weight
			delta = self.K_STDP * self.stdp_pre[delay]
			w += delta 
		if mode == "post":
			# bad input #m, decrease weight
			delta = self.K_STDP * self.stdp_post[delay]
			w -= delta
		if w >= self.WMAX:
			w = self.WMAX
		# w should not be zero - the input will never raised
		if w < 0.01:
			w = 0.01
		return w
	
