#!~/bike-computer/.venv/bin python

import serial
import serial.tools.list_ports
from multiprocessing import Process
import subprocess
import tkinter as tk
import dashboard
import interpreter

def runScript(script_name):
	subprocess.run(["python",script_name])

if __name__ == '__main__':
	p = Process(target=runScript, args=('/home/pi/bike-computer/firmware/camera.py',))
	p.start()

	ports = serial.tools.list_ports.comports()
	for port, desc, hwid in sorted(ports):
		if desc=="Arduino Micro":
			print("{}: {} [{}]".format(port, desc, hwid))
			break
	arduino = serial.Serial(port, 115200, timeout=0.1, write_timeout=0)
	path = '~/bike-computer/data/'
	intrptr = interpreter.Interpreter(arduino,path)

	root = tk.Tk()
	dbg = dashboard.DashboardGUI(root, intrptr)

	root.overrideredirect(True)
	root.after(1, dbg.update_display)
	root.mainloop()
