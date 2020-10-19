class DashboardGUI:
	def __init__(self, master, arduino):
		self.master = master
		self.arduino = arduino

		h = 316
		w = 480

		self.top_bar_canvas = tk.Canvas(master,bg="black",height=h,width=w/20)
		self.top_bar_canvas.grid(row=0,column=0,rowspan=2)
		self.time_text = self.top_bar_canvas.create_text(12,h/2,text="00:00 AM",
													angle=90,
													fill='white',
													font=('Helvetica', '12', 'bold'))
		self.sat_num_text = self.top_bar_canvas.create_text(12,h/5,text="0 SAT",
											angle=90,
											fill='white',
											font=('Helvetica', '12', 'bold'))

		self.right_arrow_points = [40,0,80,40,60,40,60,100,20,100,20,40,0,40]
		self.left_arrow_points = [40,h/2,80,h/2-40,60,h/2-40,60,h/2-100,20,h/2-100,20,h/2-40,0,h/2-40]

		self.right_signal_canvas = tk.Canvas(master,bg="black", height=h/2, width=w/6)
		self.right_signal_canvas.grid(row=0,column=1)
		self.right_arrow = self.right_signal_canvas.create_polygon(self.right_arrow_points, fill="red",state="hidden")
		self.right_arrow_on = False
		self.mph_text = self.right_signal_canvas.create_text(40,80,text="MPH",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '30', 'bold'))

		self.left_signal_canvas = tk.Canvas(master,bg="black", height=h/2, width=w/6)
		self.left_signal_canvas.grid(row=1,column=1) 
		self.left_arrow = self.left_signal_canvas.create_polygon(self.left_arrow_points, fill="red",state="hidden")
		self.left_arrow_on = False
		self.rpm_text = self.left_signal_canvas.create_text(40,80,text="RPM",
												   angle=90,
												   fill='white',
												   font=('Helvetica', '30', 'bold'))

		self.speed_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.speed_canvas.grid(row=0,column=2)
		self.speed_text = self.speed_canvas.create_text(40,80,text="0.0",
											    angle=90,
											    fill='yellow',
											    font=('Helvetica', '50', 'bold'))
		self.speed = 0.0

		self.cadence_canvas = tk.Canvas(master,height=h/2,width=w/6,bg='black')
		self.cadence_canvas.grid(row=1,column=2)
		self.cadence_text = self.cadence_canvas.create_text(40,80,text="0.0",
												   angle=90,
												   fill='yellow',
												   font=('Helvetica', '50', 'bold'))
		self.cadence = 0.0

		self.record_button = CustomButton(master, width=60,height=60, fill="white",command=self.record)
		self.record_button.grid(row=0,column=3,rowspan=2)
		self.record_started = False
		self.columns = ["satellites","latitude","longitude","timestamp","altitude",
				 "r blink state","l blink state","pedal rpm","wheel rpm",
				 "accx","accy","accz","magx","magy","magz"]
		self.data_filename = "data.csv"
		pd.DataFrame(columns=self.columns).to_csv(self.data_filename,mode='a')

		self.close_button = CustomButton(master, height=60,width=60,fill="red", X=True,command=self.quit_program)
		self.close_button.grid(row=0,column=4,rowspan=2)
		self.quit = False

		self.message_frame = [None,None,None,None,None,None,None,None,None,None,None,None,None,None,None,None]
		self.break_read = False

	def record(self):
		if not self.record_started:
			self.record_started = True
			self.record_button.itemconfigure(self.record_button.oval,fill="red")
			self.data_filename = path + time.strftime("%Y_%m_%d_%I_%M_%p", time.localtime())+".csv"
			print("RECORDING...")
		else:
			self.record_started = False
			self.record_button.itemconfigure(self.record_button.oval,fill="white")
			print("DATA RECORDED TO %s"%self.data_filename)

	def quit_program(self):
		self.quit = True
		sys.exit()

	def readSerial(self):
		while not self.quit:
			message_frame = []
			msg_start = False
			msg_end = False
			message_bytes = self.arduino.readline()
			#print(message_bytes)
			if message_bytes:
				for data in message_bytes.split(b","):
					if data==b"\r\n":
						msg_end=True
					elif msg_start and not msg_end:
						message_frame.append(data)
					elif data==b"HEAD":
						msg_start=True
					else:
						print("unknown data: ",data)
				if len(message_frame)==15:
					self.message_frame = message_frame
					#print(message_frame)
			if msg_start and msg_end:
				self.calculate()
			self.update_display()
			self.master.update()
			self.arduino.flush()
			#self.arduino.flushInput()
			self.arduino.flushOutput()


	def calculate(self):
		self.cadence = float(self.message_frame[7].decode("utf-8"))
		self.speed = float(self.message_frame[8].decode("utf-8"))*0.00134*60
		if self.record_started:
			pd.DataFrame([self.message_frame],columns=self.columns).to_csv(self.data_filename,mode='a',header=False)	
		self.top_bar_canvas.itemconfigure(self.sat_num_text,text=self.message_frame[0].decode("utf-8")+" SAT")

	def update_display(self):
		self.top_bar_canvas.itemconfigure(self.time_text,text=time.strftime("%I:%M %p", time.localtime()))
		if self.speed<50:
			if self.speed>25:
				self.speed_canvas.itemconfigure(self.speed_text,text='%.1f'%self.speed,fill="red")
			else:
				self.speed_canvas.itemconfigure(self.speed_text,text='%.1f'%self.speed,fill="yellow")
		if self.cadence<100:
			self.cadence_canvas.itemconfigure(self.cadence_text,text='%.0f'%self.cadence)
		if self.message_frame[5]==b"1":
			self.right_signal_canvas.itemconfig(self.right_arrow,state="normal")
			self.right_signal_canvas.itemconfig(self.mph_text,state="hidden")
		else:
			self.right_signal_canvas.itemconfig(self.right_arrow,state="hidden")
			self.right_signal_canvas.itemconfig(self.mph_text,state="normal")
		if self.message_frame[6]==b"1":
			self.left_signal_canvas.itemconfig(self.left_arrow,state="normal")
			self.left_signal_canvas.itemconfig(self.rpm_text,state="hidden")
		else:
			self.left_signal_canvas.itemconfig(self.left_arrow,state="hidden")
			self.left_signal_canvas.itemconfig(self.rpm_text,state="normal")

class CustomButton(tk.Canvas):
	def __init__(self, parent, width, height, fill,X=False, command=None):
		tk.Canvas.__init__(self, parent, borderwidth=1, highlightthickness=0)
		self.command = command
		self.fill=fill

		padding = 4
		self.oval = self.create_oval((padding,padding,
		  width+padding, height+padding), outline="black", fill=fill)
		if X:
			self.text = self.create_text(34,34,text="X",angle=90,
												fill='black',
												font=('Arial', '50'))
		else:
			self.square = self.create_rectangle((20,20,48,48),fill="red")

		(x0,y0,x1,y1)  = self.bbox("all")
		width = (x1-x0) + padding
		height = (y1-y0) + padding
		self.configure(width=width, height=height)
		self.bind("<ButtonPress-1>", self._on_press)
		self.bind("<ButtonRelease-1>", self._on_release)

	def _on_press(self, event):
		self.itemconfigure(self.oval,fill=self.fill)

	def _on_release(self, event):
		self.itemconfigure(self.oval,fill="red")
		if self.command is not None:
			self.command()