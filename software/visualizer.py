import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from matplotlib.collections import LineCollection

#from mpl_toolkits.basemap import Basemap

import numpy as np 
import pandas as pd 
import os

import osmnx as ox
import tkinter as tk
import sqlite3

import datetime

class Visualizer():
	def __init__(self,master,path):
		self.master = master
		self.path = path
		self.files = [file for file in os.listdir(os.path.expanduser(path)) if file.endswith(".sqlite")]
		#print("found %i data files"%len(self.files),self.files)

		h = 700
		w = 1000

		self.files_listbox = tk.Listbox(master,bg="white",width=20)
		self.files_listbox.grid(row=0,column=2,padx=20, pady=20) 
		[self.files_listbox.insert(i,datetime.datetime.strptime(file,"%Y-%m-%d-%I-%M-%S-%p-data.sqlite").strftime("%m/%d/%Y %I:%M %p")) for i,file in enumerate(self.files)]
		self.files_listbox.bind('<<ListboxSelect>>', self.selectFileEvent)

	def getFileBikeData(self,file):
		db = sqlite3.connect(self.path+file)
		df = pd.read_sql_query("SELECT * FROM bikeData", db)
		return df

	def selectFileEvent(self,event):
		w = event.widget
		index = w.curselection()[0]
		file = self.files[index]
		self.drawMap(file)
		#self.drawGPSPath(ax,file)

	def drawMap(self,file):
		df = self.getFileBikeData(file)
		df['LAT'] = df['LAT'].replace(0, np.nan)
		df['LON'] = df['LON'].replace(0, np.nan)

		max_lat = np.max(df['LAT'])
		min_lat = np.min(df['LAT']) #assuming we are only in northern hemisphere, no need to deal with negatives here
		max_lon = np.max(df['LON'])
		min_lon = np.min(df['LON'])

		mid_lat = ( max_lat - min_lat ) / 2 + min_lat
		mid_lon = ( max_lon - min_lon ) / 2 + min_lon
		radius = 1.2*max( [ self.latlong2meters((min_lat,min_lon),(min_lat,max_lon))/2,
						self.latlong2meters((min_lat,0),(max_lat,0))/2 ] ) #use the longer of the lat / lon spans to determine bbx radius

		print(max_lat,min_lat,max_lon,min_lon,radius)

		timestamps = [datetime.datetime.strptime("%d%d%d%d%d%d"%(df["YEA"][i],df["MON"][i],df["DAY"][i],
									df["HOU"][i],df["MIN"][i],df["SEC"][i]),"%Y%m%d%H%M%S").timestamp() for i in range(len(df))]

		G = ox.graph_from_point((mid_lat, mid_lon), dist= radius, network_type='all',)
		fig,ax = ox.plot_graph(G,show=False, close=False, figsize = (8,6), node_size=0, edge_color = "w", edge_linewidth=1)

		bbox = [min_lat,max_lat,min_lon,max_lon]
		
		# fig = plt.figure()

		# m = Basemap(projection='merc',llcrnrlat=bbox[0],urcrnrlat=bbox[1],\
		# 			llcrnrlon=bbox[2],urcrnrlon=bbox[3],resolution='i')
		
		cmap = df["ALT"]

		points = np.array([df["LON"], df["LAT"]]).T.reshape(-1, 1, 2)
		segments = np.concatenate([points[:-1], points[1:]], axis=1)
		norm = plt.Normalize(min(cmap), max(cmap))
		#norm = plt.Normalize(5, 30)

		lc = LineCollection(segments, cmap='winter', norm=norm)
		lc.set_array(cmap)
		lc.set_linewidth(3)
		line = ax.add_collection(lc)

		#ax.plot(df["LON"],df["LAT"],'r-')

		canvas = FigureCanvasTkAgg(fig, master=self.master)
		canvas.get_tk_widget().grid(row=0,column=0)
		canvas.draw()

		# return fig,ax,canvas

	def latlong2meters(self,latlong1,latlong2):
		# approximate radius of earth in m
		R = 6373000.0
		dlon = np.radians(latlong2[1])-np.radians(latlong1[1])
		dlat = np.radians(latlong2[0])-np.radians(latlong1[0])
		a = np.sin(dlat / 2)**2 + np.cos(latlong1[0]) * np.cos(latlong2[0]) * np.sin(dlon / 2)**2
		c = 2 * np.arctan2(np.sqrt(a), np.sqrt(1 - a))
		return R * c

	def drawGPSPath(self, ax, file):
		df = self.getFileBikeData(file)
		#plt.show()
