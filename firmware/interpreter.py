#!~/bike-computer/.venv/bin python

import time
import datetime
import pandas as pd
import pytz
import numpy as np
import sqlite3

UTC = pytz.utc 
PST = pytz.timezone("America/Los_Angeles")

class Interpreter():
	def __init__(self,arduino,path="~/"):
		self.arduino = arduino
		self.headers = ["YEA","MON","DAY","HOU","MIN","SEC","SAT","LAT","LON","ALT","PED","WHE","BLI","IMU","END"]
		self.current_data = []
		for header in self.headers[:-1]:
			setattr(self,header,-1)
			self.current_data.append(getattr(self,header))

		self.path = "/home/pi/bike-computer/data/"
		self.savefile = "data.sqlite"
		self.dbCreated = False
		self.columns = ["YEA","MON","DAY","HOU","MIN","SEC","SAT","LAT","LON","ALT","PED","WHE","BLI","IMU-MS","IMU-AX","IMU-AY","IMU-AZ","IMU-MX","IMU-MY","IMU-MZ"]

		self.bikeDataAdd_sql = ''' INSERT INTO bikeData(YEA,MON,DAY,HOU,MIN,SEC,SAT,LAT,LON,ALT,PED,WHE,BLI) 
								   VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?) '''	
		self.imuDataAdd_sql = ''' INSERT INTO imuData(YEA,MON,DAY,HOU,MIN,SEC,MS,AX,AY,AZ,MX,MY,MZ) 
								   VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?) '''
		self.bikeDataCreate_sql = """ CREATE TABLE IF NOT EXISTS bikeData (
                                        YEA integer,MON integer,DAY integer,HOU integer,MIN integer,SEC integer,
                                        SAT integer,LAT real,LON real,ALT real,PED real,WHE real,BLI integer
                                    ); """
		self.imuDataCreate_sql = """ CREATE TABLE IF NOT EXISTS imuData (
                                        YEA integer,MON integer,DAY integer,HOU integer,MIN integer,SEC integer,
                                        MS integer,AX real,AY real,AZ real,MX real,MY real,MZ real
                                    ); """                  
		self.bikeDataGetSpeed_sql=""" SELECT WHE FROM bikeData; """    
		
		self.sendSignal("~")
		print("sent start signal")
		self.lights = 0
		
		self.firstMsgRcvd = False
		self.TimeLock = False
		self.GPSLock = False
		self.Moving = False
		self.Pedaling = False

	def sendSignal(self,signal):
		self.arduino.write(signal.encode())
		time.sleep(0.1)

	def readSerial(self):
		msg_bytes = self.arduino.readline()
		msg_array = msg_bytes.decode("utf-8").split(",")
		msg_len = len(msg_array)
		try:
			header_index = self.headers.index(msg_array[0])
			ending_index = msg_array.index("\r\n")
		except:
			print("didn't understand message: ",msg_bytes)
			return -1,[-1]
		if msg_len>3:
			try:
				return header_index,[float(i) for i in msg_array[1:ending_index]]
			except:
				print("didn't understand message: ",msg_bytes)
				return -1,[-1]
		elif msg_len>2:
			try:
				return header_index,float(msg_array[1])
			except:
				print("didn't understand message: ",msg_bytes)
				return -1,[-1]
		else:
			return header_index,None

	def updateData(self,lights=0):
		header_index,data = self.readSerial()
		if header_index not in (-1,14):
			self.current_data[header_index] = data
			setattr(self,self.headers[header_index],data)
		elif header_index == 14:
			self.firstMsgRcvd = True
			self.PREV_SEC = self.SEC
			self.PREV_MS = self.IMU[0]

		# self.calculateSpeed()
		# self.calculateDistance()
		# self.calculateAvgSpeed()
		# self.calculateIncline()

		self.checkTimeLock()
		self.checkGPSLock()
		self.checkMoving()
		self.checkPedaling()

		#print(self.TimeLock,self.GPSLock,self.Moving,self.Pedaling,self.firstMsgRcvd)

		self.saveData()

		if lights!=self.lights:
			if lights:
				self.sendSignal("+")
			else:
				self.sendSignal("-")
			self.lights=lights

	def getSpeed(self):
		return self.WHE*0.00134*60

	def getDistance(self):
		return 0.0

	def getAvgSpeed(self):
		if self.dbCreated:
			self.cur.execute(self.bikeDataGetSpeed_sql)
			a = self.cur.fetchall()
			if len(a)>0:
				return np.mean(a)*0.00134*60
		return 0.0

	def getIncline(self):
		return 0.0

	def getDirection(self):
		return "N"

	def getDisplayTimeString(self):
		if self.TimeLock:
			display_time_string = self.time_obj.strftime("%m/%d/%y         %I:%M %p")
			return display_time_string
		return None

	def checkTimeLock(self):
		# Determine if there is accurate time data being fed by GPS by trying to convert the data to a datetime object
		# At the same time, store the result into a timezone converted time_obj
		try:
			date_string = "%d-%d-%d-%d-%d-%d"%(self.YEA,self.MON,self.DAY,self.HOU,self.MIN,self.SEC)
			self.time_obj = datetime.datetime.strptime(date_string,"%Y-%m-%d-%H-%M-%S").replace(tzinfo=UTC).astimezone(PST)
			self.TimeLock = True
		except:
			self.TimeLock = False

	def checkGPSLock(self):
		self.GPSLock = self.SAT>=3

	def checkMoving(self):
		self.Moving = self.WHE!=0

	def checkPedaling(self):
		self.Pedaling = self.PED!=0

	def saveData(self):
		if self.TimeLock and not self.dbCreated:
			self.db = sqlite3.connect(self.path+self.time_obj.strftime("%Y-%m-%d-%I-%M-%S-%p-")+self.savefile)
			self.cur = self.db.cursor()
			self.dbCreated = True
			self.cur.execute(self.bikeDataCreate_sql)
			self.cur.execute(self.imuDataCreate_sql)

		if self.TimeLock and self.dbCreated and self.firstMsgRcvd and self.Moving:# self.GPSLock:
			if self.SEC != self.PREV_SEC:
				self.cur.execute(self.bikeDataAdd_sql,self.current_data[:-1])
				self.db.commit()
				self.PREV_SEC = self.SEC
			# if self.IMU[0] != self.PREV_MS: #tons of issues with getting IMU data transfer over serial...come back to this
			# 	self.db.cursor().execute(self.imuDataAdd_sql,self.current_data[:6]+self.current_data[-1])
			# 	self.db.commit()
			# 	self.PREV_MS = self.IMU[0]

if __name__ == '__main__':
	import serial
	import serial.tools.list_ports
	
	ports = serial.tools.list_ports.comports()
	for port, desc, hwid in sorted(ports):
		if desc=="Arduino Micro":
			print("{}: {} [{}]".format(port, desc, hwid))
			break
	arduino = serial.Serial(port, 115200, timeout=0.1, write_timeout=0)
	path = '~/bike-computer/data/'
	interpreter = Interpreter(arduino,path)
	while True:
		interpreter.updateData()


