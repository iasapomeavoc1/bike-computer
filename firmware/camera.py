#!~/bike-computer/.venv/bin python

import picamera 
import time
import os

path = '/home/pi/bike-computer/data/'
existing_videos = [f for f in os.listdir(path) if f.endswith('.h264') and os.path.isfile(os.path.join(path, f))]
total_memory_used = 0
for video_file in existing_videos:
	total_memory_used += os.path.getsize(path+video_file)
	if total_memory_used>3500000000: #3.5GB
		os.remove(path+existing_videos[0])




with picamera.PiCamera() as camera:
	camera.resolution = (1920, 1080)
	while True:
		filename = path+time.strftime("%Y_%m_%d_%I_%M_%p", time.localtime())+".h264"
		camera.start_recording(filename)
		camera.wait_recording(600)
		camera.stop_recording()
