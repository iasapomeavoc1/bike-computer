#!~/bike-computer/.venv/bin python

import picamera 
import datetime

path = '/home/pi/bike-computer/data/'

with picamera.PiCamera(resolution=(1640,1232),framerate=30) as camera:
	try:
		while True:
			#start_time = datetime.datetime.now().timestamp()
		    for filename in camera.record_sequence(path+'%d.h264' % i for i in range(1, 6)):
		        camera.wait_recording(300)
	finally:
		camera.stop_recording()

