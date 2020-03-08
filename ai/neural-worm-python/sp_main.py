#!/usr/bin/python3
# -*- coding: utf8 -*-

# sp_main.py

import numpy as np
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import sp_var as v
# neuron class
import sp_class as c

def generate_poisson(rate):
	s = np.zeros(v.N)
	t = 0
	for i in range(v.N):
		t = t -np.log(1.0 - np.random.random()) / rate
		if t >= v.N:
			break
		s[int(t)] = 1
	return s

# main program class
class Mission:

	def __init__(self, M, N):
		self.M = M
		self.N = N
		self.robot = c.Robot()
		# init 2 neurons: right and left
		self.neuronL = c.Neuron(v.M,v.N,"L")
		self.neuronR = c.Neuron(v.M,v.N,"R")
		# array to store 2D signals for each neuron:
		self.signalL = np.zeros(v.M, dtype='int')
		self.signalR = np.zeros(v.M, dtype='int')
		# simulate sensors 
		#self.poisson = generate_poisson(1/20)
		self.poisson = generate_poisson(1/10)
		# platform angle rotation
		self.angle = 0 

		# platform coordinates
		self.x = 0
		self.y = 0

	# called in cycle with incremented time t
	def update(self,t):

		# reference signal
		po = self.poisson[t] 
		self.signalL[0] = po 
		self.signalR[0] = po 

		sensor = self.robot.getSensor(t)
		isens = 0
		# sensor spikes delay depending on sensors angle
		if sensor < 0:
			self.signalL[1] = 0 
			self.signalR[1] = self.poisson[t+isens] 
		else:
			self.signalL[1] = self.poisson[t+isens]
			self.signalR[1] = 0 

		spikesL = self.neuronL.update(self.signalL,t)
		spikesR = self.neuronR.update(self.signalR,t)
		
		if spikesL == 1:
			self.robot.motorL()

		if spikesR == 1:
			self.robot.motorR()

		self.x,self.y,self.angle = self.robot.getPos()

		return self.x,self.y,self.angle

np.set_printoptions(precision=2)
#======================
# Graphics staff
#====================== 
app = QtGui.QApplication([])
win = pg.GraphicsWindow(title="")
win.resize(800,800)
win.setWindowTitle('Neural Worm Travel')
pg.setConfigOptions(antialias=True)
scatter = win.addPlot(title="Seeking food worm track")

plt_curve = scatter.plot([0], [1],pen = None,symbolBrush=(0,0,255),pxMode=False)
plt_lin = scatter.plot([0], [1],pen = (1,3),pxMode=False)
plt_trk = scatter.plot([0], [1],pen = None,symbolBrush=(50,50,50),symbolSize=2,pxMode=False)
xt = [0]
yt = [0]
#====================== 

# main object
mission = Mission(v.M,v.N)

ptr = 0
t = 0
def step():
	global t, plt_curve, plt_lin, plt_trk, xt, yt, ptr, scatter, mission 
	if ptr == 0:
		# prepare picture in first frame
		plt_curve.setData([-500,500],[-500,500])
		scatter.enableAutoRange('xy', False)  ## stop auto-scaling after the first data set is plotted
	else:
		# find robot position and direction 
		x,y,angle = mission.update(t)
		xx, yy = int(x),int(y)
		if xx != xt[-1] or yy != yt[-1]:
			# append to track for drawing
			xt.append(xx)
			yt.append(yy)
		# draw track
		plt_trk.setData(xt,yt)
		# draw direction
		plt_lin.setData([xx,xx + 20*np.cos(angle)],[yy,yy+20*np.sin(angle)])
		plt_curve.setData([xx],[yy])
		
	t += 1
	ptr += 1

	if t >= v.N:
		exit(0)

timer = QtCore.QTimer()
timer.timeout.connect(step)
timer.start(1)

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
