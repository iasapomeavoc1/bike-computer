#!~/bike-computer/.venv/bin python

# main python script to run on raspi 
# takes in data from serial communication with Arduino
# displays data on GUI 
# logs data to logging files
# runs GPS map display + navigation GUI
# logs camera data to video files

import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import LEFT, RIGHT, TOP, BOTTOM
import time
import pandas as pd
import sys
import dashboard

path = "~/bike-computer/data/"

ports = serial.tools.list_ports.comports()
for port, desc, hwid in sorted(ports):
	#print("{}: {} [{}]".format(port, desc, hwid))
	if desc=="Arduino Micro":
		break
arduino = serial.Serial(port, 115200, timeout=.1)

root = tk.Tk()
dbg = dashboard.DashboardGUI(root, arduino)

root.overrideredirect(True)
root.after(100, dbg.readSerial)
root.mainloop()
