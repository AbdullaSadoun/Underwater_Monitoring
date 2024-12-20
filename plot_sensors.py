# from serial import Serial # was import serial (mac)
#import Serial
import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
from collections import deque
import threading
import time
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import re

class SensorPlotter:
    # def __init__(self, port='COM24', baud_rate=115200):
    def __init__(self, port='/dev/tty.usbmodem1303', baud_rate=115200): # for mac (/dev/tty.usbmodem1303)
    
        self.serial = serial.Serial(port, baud_rate)
        self.running = True
        self.last_polling_print = 0
        self.last_plot_update = time.time()
        self.plot_update_interval = 10  # Update plots every 60 seconds
        self.started = False  # Track if START has been sent
        
        # Create main window
        self.root = tk.Tk()
        self.root.title("Sensor Data Plotter")
        
        # Data storage for each sensor
        self.sensor_data = {}
        self.sensor_names = {
            2: "Acoustic",
            3: "Pressure",
            4: "Temperature",
            5: "HallEffect",
        }
        
        # Initialize data structures for each sensor
        for sensor_id in self.sensor_names:
            self.sensor_data[sensor_id] = {
                'times': deque(maxlen=1000),  # Increased buffer size
                'values': deque(maxlen=1000)
            }
        
        # Setup plot
        self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 8))
        self.fig.tight_layout(pad=3.0)
        
        # Embed plot in tkinter window
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
        
        # Add status label
        self.status_label = tk.Label(self.root, text="Status: Waiting for data...")
        self.status_label.pack(side=tk.BOTTOM)
        
        # Start animation
        self.ani = FuncAnimation(self.fig, self.update_plot, interval=1000,  # Slower update rate
                               cache_frame_data=False)
        
    def update_plot(self, frame):
        current_time = time.time()
        
        # Only update plots every plot_update_interval seconds
        if current_time - self.last_plot_update < self.plot_update_interval:
            return
        
        self.last_plot_update = current_time
        self.status_label.config(text=f"Status: Updating plots... ({time.strftime('%H:%M:%S')})")
        
        for i, (sensor_id, data) in enumerate(self.sensor_data.items()):
            row = i // 2
            col = i % 2
            
            ax = self.axes[row, col]
            ax.clear()
            
            if len(data['times']) > 0:
                ax.plot(list(data['times']), list(data['values']))
                ax.set_title(f"{self.sensor_names[sensor_id]} Sensor")
                ax.set_xlabel('Time (s)')
                ax.set_ylabel('Value')
                
                # Set reasonable y-axis limits
                values = list(data['values'])
                if values:
                    ymin, ymax = min(values), max(values)
                    yrange = max(ymax - ymin, 1)  # Avoid division by zero
                    ax.set_ylim([ymin - 0.1*yrange, ymax + 0.1*yrange])
                
                # Show last 5 minutes of data
                times = list(data['times'])
                if times:
                    xmax = max(times)
                    ax.set_xlim([max(0, xmax - 300), xmax])
        
        self.fig.tight_layout()
        self.status_label.config(text=f"Status: Last update at {time.strftime('%H:%M:%S')}")
            
    def read_serial(self):
        while self.running:
            if self.serial.in_waiting:
                try:
                    line = self.serial.readline().decode('utf-8').strip()
                    
                    # Print all terminal output
                    print(line)
                    
                    # Auto-start when seeing "Polling"
                    if line == "Polling" and not self.started:
                        print("Detected polling, sending START command...")
                        self.send_command("START")
                        self.started = True
                    
                    # Parse sensor data from debug messages
                    if line.startswith("Received message"):
                        # Extract values using regex
                        match = re.search(r"SensorID: (\d+), MsgID: (\d+), Params: (\d+)", line)
                        if match:
                            sensor_id = int(match.group(1))
                            msg_id = int(match.group(2))
                            value = int(match.group(3))
                            
                            if sensor_id in self.sensor_data:
                                current_time = time.time()
                                self.sensor_data[sensor_id]['times'].append(current_time)
                                self.sensor_data[sensor_id]['values'].append(value)
                                
                except UnicodeDecodeError:
                    print("Error decoding serial data")
                    pass
        self.root.after(1, self.read_serial)  # Schedule the next read after 100 ms changed to 5 ms mac

                
    def send_command(self, command):
        self.serial.write((command + '\n').encode('utf-8'))
        
    def start(self):
        self.read_serial() # mac
        self.root.mainloop() # mac

        # Start reading thread
        self.read_thread = threading.Thread(target=self.read_serial)
        self.read_thread.daemon = True
        self.read_thread.start()
        
        # Start GUI
        self.root.mainloop()
        
    def stop(self):
        self.running = False
        if hasattr(self, 'read_thread'):
            self.read_thread.join()
        self.serial.close()
        plt.close('all')

if __name__ == '__main__':
    print("Make sure PuTTY is closed before continuing...")
    input("Press Enter when ready...")
    
    plotter = SensorPlotter()
    try:
        plotter.start()
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        plotter.stop() 

'''
Mac Usage:
- View available ports: ls /dev/tty.*

pip install contourpy==1.3.1 cycler==0.12.1 fonttools==4.55.0 kiwisolver==1.4.7 matplotlib==3.9.2 numpy==2.1.3 packaging==24.2 pillow==11.0.0 pyparsing==3.2.0 pyserial==3.5 python-dateutil==2.9.0.post0 six==1.16.0 tk==0.1.0

Python 3.12.0
contourpy==1.3.1
cycler==0.12.1
fonttools==4.55.0
kiwisolver==1.4.7
matplotlib==3.9.2
numpy==2.1.3
packaging==24.2
pillow==11.0.0
pyparsing==3.2.0
pyserial==3.5
python-dateutil==2.9.0.post0
six==1.16.0
tk==0.1.0


'''