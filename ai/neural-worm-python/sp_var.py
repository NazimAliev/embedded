#!/usr/bin/python3
# -*- coding: utf8 -*-

# sp_var.py

import os,sys
import numpy as np

np.set_printoptions(precision=2)

# time samples
N = 10000000 
# number of neuron inputs
M = 2
# stdp curve koeff
K_STDP = 1.5
# stdp window
WIN = 8
# post STDP more strong than pre
stdp_pre = np.exp(-np.arange(WIN)/1.0)
stdp_post = 1.2*np.exp(-np.arange(WIN)/2.0)
# fire level
FIRE_LEVEL=1200
# input signal
signal = np.zeros((M,N), dtype='int')
# max weights
WMAX = 3.0

