#!~/bike-computer/.venv/bin python

import picamera 
import time
import os

path = '/home/pi/bike-computer/data/'
stored_files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]
total_memory_used = 0

for file in stored_files:
	total_memory_used += os.path.getsize(path+file)
	if total_memory_used>3500000000: #3.5GB
		print("MORE THAN 3.5GB USED - DELETE FILES TO CONTINUE. EXITING...") 

with picamera.PiCamera() as camera:
	camera.resolution = (1920, 1080)
	while True:
		filename = path+time.strftime("%Y_%m_%d_%I_%M_%p", time.localtime())+".h264"
		camera.start_recording(filename)
		camera.wait_recording(600)
		camera.stop_recording()
