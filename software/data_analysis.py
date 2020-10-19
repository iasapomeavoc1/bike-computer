import matplotlib.pyplot as plt  
from matplotlib.collections import LineCollection
from matplotlib import dates

import numpy as np 
import pandas as pd 
import os

path = os.path.expanduser('~/Active-Projects/bike-computer/data/')
filenames = [path+filename for filename in os.listdir(path) if filename.endswith(".csv")]
columns = ["satellites","latitude","longitude","timestamp","altitude",
		 "r blink state","l blink state","pedal rpm","wheel rpm",
		 "accx","accy","accz","magx","magy","magz"]

import osmnx as ox

G = ox.graph_from_point((37.79, -122.41), dist=100, network_type='all')
fig,ax = ox.plot_graph(G,show=False, close=False)

def plot_data(filename):
	print(filename)
	df = pd.read_csv(filename,sep="',b'",header=None)
	df.columns = columns

	df.loc[[i==None for i in df["timestamp"]],"timestamp"] = "00/00"
	df.loc[[type(i)!=str for i in df["timestamp"]],"timestamp"] = "00/00"
	df.loc[[i[3:5]=="00" for i in df["timestamp"]],"timestamp"] = "00/00"
	df["timestamp_trunc"] = df["timestamp"].map(lambda x: x[0:19])
	df.loc[[i.startswith("00/00") or len(i)!=19 for i in df["timestamp_trunc"]],"timestamp_trunc"] = np.NaN

	df["timestamp_plot"] = pd.to_datetime(df["timestamp_trunc"],format="%m/%d/%Y %H:%M:%S")
	df.loc[[i.hour==0 for i in df["timestamp_plot"]],"timestamp_plot"] = np.NaN
	df["timestamp_sec"] = (df["timestamp_plot"].dt.hour*60+df["timestamp_plot"].dt.minute)*60 + df["timestamp_plot"].dt.second + pd.to_numeric(df["timestamp"].map(lambda x: x[19:]))/1000.0

	df["latitude"] = pd.to_numeric(df["latitude"],errors='coerce')
	df.loc[[i>90 or i<-90 for i in df["latitude"]],"latitude"] = np.NaN
	df["longitude"] = pd.to_numeric(df["longitude"],errors='coerce')
	df.loc[[i>0 or i<-180 for i in df["longitude"]],"longitude"] = np.NaN
	df["satellites"] = pd.to_numeric(df["satellites"],errors='coerce')
	df.loc[[i>10 for i in df["satellites"]],"satellites"] = np.NaN
	df["altitude"] = pd.to_numeric(df["altitude"],errors='coerce')

	df["delta_miles"] = np.sqrt((df["latitude"].diff()*69.172)**2+
								 (df["longitude"].diff()*(np.cos(df["latitude"]*.01745)*69.172))**2)
	df["miles_traveled"] = np.cumsum(df["delta_miles"])
	print(np.nanmax(df["miles_traveled"]))

	df["wheel rpm"] = pd.to_numeric(df["wheel rpm"],errors='coerce')*0.00134*60
	df.loc[[i>500 for i in df["wheel rpm"]],"wheel rpm"] = np.NaN

	df = df.dropna(subset = ["altitude"])


	df["accx"] = pd.to_numeric(df["accx"],errors='coerce')
	df["accy"] = pd.to_numeric(df["accx"],errors='coerce')
	df["accz"] = pd.to_numeric(df["accx"],errors='coerce')
	df["acc_magnitude"] = np.cbrt(np.square(df["accx"])+np.square(df["accy"])+np.square(df["accz"]))


	#fig = plt.figure(figsize=(12, 6))

	fig.suptitle(filename[-23:-4])

	#plt.plot(df["timestamp_sec"],df["accz"])
	
	# print(pd.to_numeric(df["timestamp"].map(lambda x: x[19:])))
	# plt.plot(df["timestamp_plot"].dt.second,'.')# + pd.to_numeric(df["timestamp"].map(lambda x: x[19:]))/1000.0,'.')
	
	# image_name = '/Users/Roger/Active-Projects/bike-computer/data/sf_map.png'
	# extent =  [-122.5205,-122.3495,37.720,37.8106] #save this
	# im = plt.imread(image_name)
	# plt.imshow(im,extent=extent,alpha=0.8,interpolation='none')

	plt.scatter(df["longitude"],df["latitude"],c=range(len(df)),marker='.')
	#plt.colorbar(label="time")

	#ax = fig.axes[0]
	#ax.set_aspect(aspect=1.3) 

	plt.ylabel("latitude")
	plt.xlabel("longitude")

	plt.xlim([-122.5205,-122.3495])
	plt.ylim([37.720,37.8106])

	plt.show()

for filename in filenames:
	plot_data(filename)