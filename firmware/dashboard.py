import tkinter as tk

class DashboardGUI:
	def __init__(self, master, interpreter):
		self.master = master
		self.interpreter = interpreter

		h = 316
		w = 480

		self.top_bar_canvas = tk.Canvas(master,bg="black",height=h,width=w/20)
		self.top_bar_canvas.grid(row=0,column=0,rowspan=2)
		self.time_text = self.top_bar_canvas.create_text(12,0.67*h,text="IDK:IDK AM",
													angle=90,
													fill='white',
													font=('Helvetica', '12', 'bold'))
		self.sat_num_text = self.top_bar_canvas.create_text(12,0.15*h,text="0 SAT",
											angle=90,
											fill='white',
											font=('Helvetica', '12', 'bold'))

		self.speed_label_canvas = tk.Canvas(master,bg="black", height=h/2, width=w/12)
		self.speed_label_canvas.grid(row=0,column=1)
		self.speed_label_text = self.speed_label_canvas.create_text(20,80,text="SPEED (MPH)",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '15', 'bold'))

		self.rpm_label_canvas = tk.Canvas(master,bg="black", height=h/2, width=w/12)
		self.rpm_label_canvas.grid(row=1,column=1) 
		self.rpm_label_text = self.rpm_label_canvas.create_text(20,80,text="CADENCE (RPM)",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '12', 'bold'))

		self.speed_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.speed_canvas.grid(row=0,column=2)
		self.speed_text = self.speed_canvas.create_text(40,80,text="0.0",
											    angle=90,
											    fill='yellow',
											    font=('Helvetica', '50', 'bold'))

		self.cadence_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.cadence_canvas.grid(row=1,column=2)
		self.cadence_text = self.cadence_canvas.create_text(40,80,text="0.0",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))

		self.avg_speed_label_canvas = tk.Canvas(master,height=h/2,width=w/12,bg='black')
		self.avg_speed_label_canvas.grid(row=0,column=3)
		self.avg_speed_label_text = self.avg_speed_label_canvas.create_text(20,80,text="AVG SPEED",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '15', 'bold'))

		self.distance_label_canvas = tk.Canvas(master,height=h/2,width=w/12,bg='black')
		self.distance_label_canvas.grid(row=1,column=3)
		self.distance_label_text = self.distance_label_canvas.create_text(20,80,text="DISTANCE (MILES)",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '11', 'bold'))

		self.avg_speed_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.avg_speed_canvas.grid(row=0,column=4)
		self.avg_speed_text = self.avg_speed_canvas.create_text(40,80,text="0.0",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))

		self.distance_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.distance_canvas.grid(row=1,column=4)
		self.distance_text = self.distance_canvas.create_text(40,80,text="0.0",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))

		self.direction_label_canvas = tk.Canvas(master,height=h/2,width=w/12,bg='black')
		self.direction_label_canvas.grid(row=0,column=5)
		self.direction_label_text = self.direction_label_canvas.create_text(20,80,text="DIRECTION",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '15', 'bold'))

		self.incline_label_canvas = tk.Canvas(master,height=h/2,width=w/12,bg='black')
		self.incline_label_canvas.grid(row=1,column=5)
		self.incline_label_text = self.incline_label_canvas.create_text(20,80,text="INCLINE (DEG)",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '13', 'bold'))

		self.direction_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.direction_canvas.grid(row=0,column=6)
		self.direction_text = self.direction_canvas.create_text(40,80,text="N",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))

		self.incline_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.incline_canvas.grid(row=1,column=6)
		self.incline_text = self.incline_canvas.create_text(40,80,text="0.0",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))
		self.lights_button = LatchingButton(master, width=60,height=60,fills=("white","red"),command=self.toggle_lights)
		self.lights_button.grid(row=0,column=7,rowspan=1)
		self.lights = 0

	def toggle_lights(self):
		self.lights = 1 - self.lights

	def update_display(self):
		self.interpreter.updateData(self.lights)

		self.top_bar_canvas.itemconfigure(self.sat_num_text,text='%.1f SAT'%self.interpreter.SAT)

		self.top_bar_canvas.itemconfigure(self.time_text,text=self.interpreter.getDisplayTimeString())

		if self.interpreter.getSpeed()>25:
			self.speed_canvas.itemconfigure(self.speed_text,text='%.1f'%self.interpreter.getSpeed(),fill="red")
		else:
			self.speed_canvas.itemconfigure(self.speed_text,text='%.1f'%self.interpreter.getSpeed(),fill="yellow")

		self.cadence_canvas.itemconfigure(self.cadence_text,text='%.0f'%self.interpreter.PED)

		self.avg_speed_canvas.itemconfigure(self.avg_speed_text,text='%.0f'%self.interpreter.getAvgSpeed())

		self.distance_canvas.itemconfigure(self.distance_text,text='%.0f'%self.interpreter.getDistance())

		self.direction_canvas.itemconfigure(self.direction_text,text=self.interpreter.getDirection())

		self.incline_canvas.itemconfigure(self.incline_text,text='%.0f'%self.interpreter.getIncline())

		self.master.after(1,self.update_display) #this ensures this process continually repeats

class LatchingButton(tk.Canvas):
	def __init__(self, parent, width, height, fills, command=None):
		tk.Canvas.__init__(self, parent, borderwidth=1, highlightthickness=0)
		self.command = command
		self.fills=fills
		self.fill_index = 0

		padding = 4
		self.oval = self.create_oval((padding,padding, width+padding, height+padding),
									  outline="black",
									  fill=self.fills[self.fill_index])
		(x0,y0,x1,y1)  = self.bbox("all")
		width = (x1-x0) + padding
		height = (y1-y0) + padding
		self.configure(width=width, height=height)
		self.bind("<ButtonPress-1>", self._on_press)
		self.bind("<ButtonRelease-1>", self._on_release)

	def _on_press(self, event):
		pass

	def _on_release(self, event):
		self.fill_index = 1 - self.fill_index
		self.itemconfigure(self.oval,fill=self.fills[self.fill_index])
		if self.command is not None:
			self.command()

if __name__ == '__main__':
	import serial
	import serial.tools.list_ports
	import interpreter
	
	ports = serial.tools.list_ports.comports()
	for port, desc, hwid in sorted(ports):
		if desc=="Arduino Micro":
			print("{}: {} [{}]".format(port, desc, hwid))
			break
	arduino = serial.Serial(port, 115200, timeout=0.1, write_timeout=0)
	path = '~/bike-computer/data/'
	intrptr = interpreter.Interpreter(arduino,path)

	root = tk.Tk()
	dbg = DashboardGUI(root, intrptr)

	root.overrideredirect(True)
	root.after(10, dbg.update_display)
	root.mainloop()