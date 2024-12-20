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
from queue import Queue
import matplotlib
matplotlib.use('TkAgg')  # Explicitly set the backend

class SensorPlotter:
    def __init__(self, port=None, baud_rate=115200):
        # If no port specified, try to find the first available USB modem
        if port is None:
            import glob
            ports = glob.glob('/dev/tty.usbmodem*')
            if not ports:
                raise Exception("No USB modem ports found! Connect your device and try again.")
            port = ports[0]
            print(f"Using port: {port}")
        
        self.serial = serial.Serial(port, baud_rate)
        self.running = True
        self.last_plot_update = time.time()
        self.plot_update_interval = 10
        self.started = False
        self.data_queue = Queue()
        
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
                'times': deque(maxlen=1000),
                'values': deque(maxlen=1000)
            }
        
        self.setup_gui()
        
    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("Sensor Data Plotter")
        
        # Setup plot
        self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 8))
        self.fig.tight_layout(pad=3.0)
        
        # Embed plot in tkinter window
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
        
        # Add status label
        self.status_label = tk.Label(self.root, text="Status: Waiting for data...")
        self.status_label.pack(side=tk.BOTTOM)
        
        # Schedule the first update
        self.root.after(1000, self.update_gui)
        
    def update_gui(self):
        if not self.running:
            return
            
        # Process any queued data
        while not self.data_queue.empty():
            sensor_id, value, timestamp = self.data_queue.get()
            if sensor_id in self.sensor_data:
                self.sensor_data[sensor_id]['times'].append(timestamp)
                self.sensor_data[sensor_id]['values'].append(value)
        
        current_time = time.time()
        if current_time - self.last_plot_update >= self.plot_update_interval:
            self.last_plot_update = current_time
            self.update_plot()
            
        # Schedule the next update
        self.root.after(100, self.update_gui)
        
    def update_plot(self):
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
                
                values = list(data['values'])
                if values:
                    ymin, ymax = min(values), max(values)
                    yrange = max(ymax - ymin, 1)
                    ax.set_ylim([ymin - 0.1*yrange, ymax + 0.1*yrange])
                
                times = list(data['times'])
                if times:
                    xmax = max(times)
                    ax.set_xlim([max(0, xmax - 300), xmax])
        
        self.fig.tight_layout()
        try:
            self.canvas.draw()
        except tk.TclError:
            self.running = False
            
    def read_serial(self):
        while self.running:
            if self.serial.in_waiting:
                try:
                    line = self.serial.readline().decode('utf-8').strip()
                    print(line)  # Print all terminal output
                    
                    if line == "Polling" and not self.started:
                        print("Detected polling, sending START command...")
                        self.send_command("START")
                        self.started = True
                    
                    if line.startswith("Received message"):
                        match = re.search(r"SensorID: (\d+), MsgID: (\d+), Params: (\d+)", line)
                        if match:
                            sensor_id = int(match.group(1))
                            value = int(match.group(3))
                            current_time = time.time()
                            self.data_queue.put((sensor_id, value, current_time))
                                
                except UnicodeDecodeError:
                    print("Error decoding serial data")
            time.sleep(0.0000001)  # Small delay to prevent CPU overuse
                
    def send_command(self, command):
        self.serial.write((command + '\n').encode('utf-8'))
        
    def start(self):
        # Start reading thread
        self.read_thread = threading.Thread(target=self.read_serial)
        self.read_thread.daemon = True
        self.read_thread.start()
        
        # Start GUI in main thread
        self.root.mainloop()
        
    def stop(self):
        self.running = False
        if hasattr(self, 'read_thread'):
            self.read_thread.join(timeout=1.0)
        self.serial.close()
        plt.close('all')
        if self.root:
            self.root.quit()

if __name__ == '__main__':
    print("Make sure no other programs are using the serial port...")
    input("Press Enter when ready...")
    
    try:
        plotter = SensorPlotter()  # Will auto-detect USB modem port
        plotter.start()
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'plotter' in locals():
            plotter.stop()