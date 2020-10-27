import tkinter as tk
import visualizer

if __name__ == '__main__':
	root = tk.Tk()
	path = "/Users/Roger/Projects/bike-computer/data/"
	vis = visualizer.Visualizer(root,path)

	#root.attributes('-fullscreen',True)
	root.overrideredirect(True)
	#root.after(1, dbg.update_display)
	root.mainloop()

