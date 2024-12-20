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
import customtkinter as ctk

class SensorPlotter:
    # Define default threshold constants
    DEFAULT_THRESHOLDS = {
        'Acoustic': {'low': 50, 'high': 80},
        'Pressure': {'low': 100, 'high': 200}, 
        'HallEffect': {'low': 100, 'high': 150}
    }
    
    def __init__(self, port=None, baud_rate=115200):
        # Get user threshold preferences
        self.THRESHOLDS = self.get_user_thresholds()
        
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
        self.plot_update_interval = 1 # was 10s
        self.started = False
        self.data_queue = Queue()
        
        # Data storage for each sensor
        self.sensor_data = {}
        self.sensor_names = {
            2: "Acoustic",
            3: "Pressure",
            4: "Temperature",
            5: "HallEffect/Flowrate",
        }

        self.alerts = {
            2: {'status': 'Normal', 'message': ''},
            3: {'status': 'Normal', 'message': ''},
            4: {'status': 'Normal', 'message': ''}, 
            5: {'status': 'Normal', 'message': ''}
        }
        
        # Initialize data structures for each sensor
        for sensor_id in self.sensor_names:
            self.sensor_data[sensor_id] = {
                'times': deque(maxlen=1000),
                'values': deque(maxlen=1000)
            }
        
        self.setup_gui()
        
    def setup_gui(self):
        # Set theme and color
        ctk.set_appearance_mode("dark")  # Options: "dark", "light"
        ctk.set_default_color_theme("blue")  # Options: "blue", "green", "dark-blue"
        
        # Create the main window
        self.root = ctk.CTk()
        self.root.title("Subsea Sensor Monitoring System")
        self.root.geometry("1200x800")
        
        # Create main container
        self.main_frame = ctk.CTkFrame(self.root)
        self.main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Create left sidebar for controls
        self.sidebar = ctk.CTkFrame(self.main_frame, width=200)
        self.sidebar.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)
        
        # Add title to sidebar
        ctk.CTkLabel(self.sidebar, text="Control Panel", font=("Helvetica", 16, "bold")).pack(pady=10)
        
        # Add single STOP button with red color
        self.stop_button = ctk.CTkButton(
            self.sidebar, 
            text="STOP", 
            command=self.stop,
            fg_color="red",
            hover_color="darkred",
            font=("Helvetica", 14, "bold"),
            height=40
        )
        self.stop_button.pack(pady=10)
        
        # Add threshold controls
        self.threshold_frame = ctk.CTkFrame(self.sidebar)
        self.threshold_frame.pack(pady=10, padx=5, fill=tk.X)
        
        ctk.CTkLabel(self.threshold_frame, 
                    text="Threshold Controls", 
                    font=("Helvetica", 14, "bold")).pack(pady=(5,10))
        
        # Add threshold adjustment sliders with labels and text boxes
        self.threshold_sliders = {}
        self.threshold_entries = {}
        
        for sensor in ['Acoustic', 'Pressure', 'HallEffect']:
            sensor_frame = ctk.CTkFrame(self.threshold_frame)
            sensor_frame.pack(pady=10, padx=5, fill=tk.X)
            
            # Sensor name label
            ctk.CTkLabel(sensor_frame, 
                        text=sensor,
                        font=("Helvetica", 12, "bold")).pack(pady=5)
            
            # Low threshold controls
            low_frame = ctk.CTkFrame(sensor_frame)
            low_frame.pack(fill=tk.X, pady=2)
            
            ctk.CTkLabel(low_frame, text="Low:", width=30).pack(side=tk.LEFT, padx=5)
            
            self.threshold_entries[f"{sensor}_low"] = ctk.CTkEntry(
                low_frame, 
                width=50,
                placeholder_text=str(self.THRESHOLDS[sensor]['low']))
            self.threshold_entries[f"{sensor}_low"].pack(side=tk.RIGHT, padx=5)
            
            self.threshold_sliders[sensor] = {
                'low': ctk.CTkSlider(
                    sensor_frame,
                    from_=0,
                    to=200,
                    number_of_steps=200,
                    command=lambda v, s=sensor: self.update_threshold_from_slider(s, 'low', v)
                )
            }
            self.threshold_sliders[sensor]['low'].pack(fill=tk.X, padx=5, pady=(0,5))
            self.threshold_sliders[sensor]['low'].set(self.THRESHOLDS[sensor]['low'])
            
            # High threshold controls
            high_frame = ctk.CTkFrame(sensor_frame)
            high_frame.pack(fill=tk.X, pady=2)
            
            ctk.CTkLabel(high_frame, text="High:", width=30).pack(side=tk.LEFT, padx=5)
            
            self.threshold_entries[f"{sensor}_high"] = ctk.CTkEntry(
                high_frame, 
                width=50,
                placeholder_text=str(self.THRESHOLDS[sensor]['high']))
            self.threshold_entries[f"{sensor}_high"].pack(side=tk.RIGHT, padx=5)
            
            self.threshold_sliders[sensor]['high'] = ctk.CTkSlider(
                sensor_frame,
                from_=0,
                to=200,
                number_of_steps=200,
                command=lambda v, s=sensor: self.update_threshold_from_slider(s, 'high', v)
            )
            self.threshold_sliders[sensor]['high'].pack(fill=tk.X, padx=5, pady=(0,5))
            self.threshold_sliders[sensor]['high'].set(self.THRESHOLDS[sensor]['high'])
            
            # Bind entry validation and update
            self.threshold_entries[f"{sensor}_low"].bind(
                '<Return>', 
                lambda e, s=sensor: self.update_threshold_from_entry(s, 'low'))
            self.threshold_entries[f"{sensor}_high"].bind(
                '<Return>', 
                lambda e, s=sensor: self.update_threshold_from_entry(s, 'high'))
        
        # Create right frame for plots
        self.plot_frame = ctk.CTkFrame(self.main_frame)
        self.plot_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Setup plot
        self.fig, self.axes = plt.subplots(2, 2, figsize=(12, 8))
        self.fig.tight_layout(pad=3.0)
        
        # Embed plot in frame
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.plot_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        # Add status bar
        self.status_label = ctk.CTkLabel(self.root, text="Status: Waiting for data...")
        self.status_label.pack(side=tk.BOTTOM, pady=5)
        
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

    def check_thresholds(self, sensor_id, value):
        sensor_name = self.sensor_names[sensor_id]
        if sensor_name in self.THRESHOLDS:
            thresholds = self.THRESHOLDS[sensor_name]
            
            if value < thresholds['low']:
                if sensor_name == 'Acoustic':
                    msg = "Normal Operation"
                elif sensor_name == 'Pressure':
                    msg = "Low Pressure - Potential Risk"
                elif sensor_name == 'HallEffect/Flowrate':
                    msg = "Low Flow - Possible Blockage"
                self.alerts[sensor_id] = {'status': 'Low', 'message': msg}
                
            elif value > thresholds['high']:
                if sensor_name == 'Acoustic':
                    msg = "Alert - Potential Risk/Error"
                elif sensor_name == 'Pressure':
                    msg = "High Pressure - Potential Risk/Error"
                elif sensor_name == 'HallEffect/Flowrate':
                    msg = "High Flow - Possible Equipment Malfunction"
                self.alerts[sensor_id] = {'status': 'High', 'message': msg}
                
            else:
                self.alerts[sensor_id] = {'status': 'Normal', 'message': 'Operating Normally'}
        
    def update_plot(self):
        for i, (sensor_id, data) in enumerate(self.sensor_data.items()):
            row = i // 2
            col = i % 2
            
            ax = self.axes[row, col]
            ax.clear()
            
            if len(data['times']) > 0:
                # Plot with better styling
                ax.plot(list(data['times']), list(data['values']), 
                       linewidth=2, color='#2596be')
                
                # Add grid
                ax.grid(True, linestyle='--', alpha=0.7)
                
                # Style the plot
                ax.set_title(f"{self.sensor_names[sensor_id]}", 
                           fontsize=12, pad=10)
                ax.set_xlabel('Time (s)', fontsize=10)
                ax.set_ylabel('Value', fontsize=10)
                
                values = list(data['values'])
                if values:
                    ymin, ymax = min(values), max(values)
                    yrange = max(ymax - ymin, 1)
                    ax.set_ylim([ymin - 0.1*yrange, ymax + 0.1*yrange])
                
                times = list(data['times'])
                if times:
                    xmax = max(times)
                    ax.set_xlim([max(0, xmax - 300), xmax])

                # Check thresholds and update status
                # Add alert coloring
                alert_status = self.alerts[sensor_id]['status']
                if alert_status == 'High':
                    ax.set_facecolor('mistyrose')
                elif alert_status == 'Low':
                    ax.set_facecolor('lightblue') 
                else:
                    ax.set_facecolor('white')
                    
                # Add alert message
                alert_msg = self.alerts[sensor_id]['message']
                ax.set_title(f"{self.sensor_names[sensor_id]}\n{alert_msg}")

        
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

                            # Check thresholds and update alerts
                            self.check_thresholds(sensor_id, value)
                                
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
        """Clean shutdown of the application"""
        self.running = False
        if hasattr(self, 'read_thread'):
            self.read_thread.join(timeout=1.0)
        self.serial.close()
        plt.close('all')
        if self.root:
            self.root.quit()
            self.root.destroy()  # Ensure window is fully closed

    def get_user_thresholds(self):
        print("\nThreshold Configuration")
        print("----------------------")
        print("1. Set custom thresholds")
        print(f"OR Press Enter to use default values:")
        print(f"  Acoustic: {self.DEFAULT_THRESHOLDS['Acoustic']['low']}-{self.DEFAULT_THRESHOLDS['Acoustic']['high']}")
        print(f"  Pressure: {self.DEFAULT_THRESHOLDS['Pressure']['low']}-{self.DEFAULT_THRESHOLDS['Pressure']['high']}")
        print(f"  HallEffect/Flow: {self.DEFAULT_THRESHOLDS['HallEffect']['low']}-{self.DEFAULT_THRESHOLDS['HallEffect']['high']}")
        
        choice = input("\nEnter your choice (1 or press Enter for defaults): ").strip()
        
        if not choice:
            return self.DEFAULT_THRESHOLDS
            
        if choice == "1":
            custom_thresholds = {}
            
            for sensor in ['Acoustic', 'Pressure', 'HallEffect']:
                print(f"\nEnter thresholds for {sensor}:")
                while True:
                    try:
                        low = int(input(f"Low threshold (default={self.DEFAULT_THRESHOLDS[sensor]['low']}): ").strip() or 
                                self.DEFAULT_THRESHOLDS[sensor]['low'])
                        high = int(input(f"High threshold (default={self.DEFAULT_THRESHOLDS[sensor]['high']}): ").strip() or 
                                 self.DEFAULT_THRESHOLDS[sensor]['high'])
                        
                        if low >= high:
                            print("Error: Low threshold must be less than high threshold. Please try again.")
                            continue
                            
                        custom_thresholds[sensor] = {'low': low, 'high': high}
                        break
                    except ValueError:
                        print("Please enter valid numbers. Try again.")
            
            return custom_thresholds
        
        return self.DEFAULT_THRESHOLDS

    def update_threshold_from_slider(self, sensor, threshold_type, value):
        """Update threshold from slider movement"""
        # Update entry
        self.threshold_entries[f"{sensor}_{threshold_type}"].delete(0, tk.END)
        self.threshold_entries[f"{sensor}_{threshold_type}"].insert(0, str(int(value)))
        
        # Update thresholds if valid
        low = self.threshold_sliders[sensor]['low'].get()
        high = self.threshold_sliders[sensor]['high'].get()
        if low < high:
            self.THRESHOLDS[sensor] = {'low': low, 'high': high}
        else:
            # Reset to previous valid values
            if threshold_type == 'low':
                self.threshold_sliders[sensor]['low'].set(self.THRESHOLDS[sensor]['low'])
            else:
                self.threshold_sliders[sensor]['high'].set(self.THRESHOLDS[sensor]['high'])
    
    def update_threshold_from_entry(self, sensor, threshold_type, event=None):
        """Update threshold from entry box"""
        try:
            value = float(self.threshold_entries[f"{sensor}_{threshold_type}"].get())
            if 0 <= value <= 200:
                # Update slider
                self.threshold_sliders[sensor][threshold_type].set(value)
                
                # Update thresholds if valid
                low = float(self.threshold_entries[f"{sensor}_low"].get())
                high = float(self.threshold_entries[f"{sensor}_high"].get())
                if low < high:
                    self.THRESHOLDS[sensor] = {'low': low, 'high': high}
                else:
                    raise ValueError("Low must be less than high")
            else:
                raise ValueError("Value must be between 0 and 200")
        except ValueError as e:
            # Reset entry to current threshold value
            self.threshold_entries[f"{sensor}_{threshold_type}"].delete(0, tk.END)
            self.threshold_entries[f"{sensor}_{threshold_type}"].insert(
                0, 
                str(self.THRESHOLDS[sensor][threshold_type]))
            print(f"Invalid input: {e}")
    
    def update_status(self, message):
        self.status_label.configure(text=f"Status: {message}")

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