"""
Python GUI & Data Logger for Arduino Fan Controller
Reads Serial data, displays a live gauge and LED, and writes data to a CSV.
"""
import tkinter as tk
import serial
import csv
import math

# --- Configuration ---
# CHANGE THIS to match your Arduino's COM port (e.g., 'COM3' on Windows, '/dev/ttyACM0' on Mac/Linux)
SERIAL_PORT = 'COM4' 
BAUD_RATE = 9600
CSV_FILENAME = 'sensor_data_log.csv'

class ArduinoMonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Arduino Fan & Servo Monitor")
        self.root.geometry("400x350")
        
        # 1. Setup UI Elements
        self.setup_ui()
        
        # 2. Initialize CSV File
        self.init_csv()
        
        # 3. Connect to Arduino
        self.serial_conn = None
        self.connect_serial()
        
        # 4. Start the update loop
        self.update_data()

    def setup_ui(self):
            """Creates the canvas for the compass/gauge and LED."""
            tk.Label(self.root, text="Live Servo Angle", font=("Arial", 14, "bold")).pack(pady=5)
            
            # Gauge Canvas
            self.canvas = tk.Canvas(self.root, width=200, height=150, bg="white")
            self.canvas.pack()
            
            # Draw the base arc for the gauge (sweeps from 0 to 165 degrees)
            self.canvas.create_arc(10, 10, 190, 190, start=0, extent=165, outline="black", style=tk.ARC, width=2)
            
            # Initialize the pointer pointing to 0 degrees (straight to the right)
            self.pointer = self.canvas.create_line(100, 100, 180, 100, fill="blue", width=3)
            
            self.angle_label = tk.Label(self.root, text="Angle: -- deg", font=("Arial", 12))
            self.angle_label.pack(pady=5)
            
            # Buzzer LED Canvas
            tk.Label(self.root, text="Buzzer State", font=("Arial", 12, "bold")).pack(pady=5)
            self.led_canvas = tk.Canvas(self.root, width=50, height=50)
            self.led_canvas.pack()
            self.led = self.led_canvas.create_oval(10, 10, 40, 40, fill="gray")

    def init_csv(self):
        """Creates the CSV file and writes the header row."""
        try:
            with open(CSV_FILENAME, mode='w', newline='') as file:
                writer = csv.writer(file)
                writer.writerow(["Time (ms)", "Angle (deg)", "Buzzer State"])
        except Exception as e:
            print(f"Error creating CSV: {e}")

    def connect_serial(self):
        """Attempts to open the serial port."""
        try:
            self.serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
            print(f"Connected to {SERIAL_PORT}")
        except serial.SerialException:
            print(f"WARNING: Could not connect to {SERIAL_PORT}. Please check the port.")

    def update_data(self):
        """Reads from serial, updates GUI, and writes to CSV continuously."""
        if self.serial_conn and self.serial_conn.in_waiting > 0:
            try:
                # Read and decode the line from Arduino
                line = self.serial_conn.readline().decode('utf-8').strip()
                
                # Ensure we have valid data (Time, Angle, Buzzer)
                if line and line.count(',') == 2:
                    time_ms, angle_str, buzzer_str = line.split(',')
                    angle = int(angle_str)
                    buzzer_state = int(buzzer_str)
                    
                    # Update GUI
                    self.update_gui(angle, buzzer_state)
                    
                    # Log to CSV
                    self.log_to_csv(time_ms, angle, buzzer_state)
                    
            except Exception as e:
                # Catch decoding or parsing errors (common when serial first connects)
                pass 
                
        # Schedule this function to run again in 50ms
        self.root.after(50, self.update_data)

    def update_gui(self, angle, buzzer_state):
        """Updates the visual elements based on new data."""
        self.angle_label.config(text=f"Angle: {angle} deg")
        
        rad = math.radians(angle)
        length = 80
        x = 100 + length * math.cos(rad)
        
        # Tkinter's Y-axis goes down, so we subtract to make the needle go up
        y = 100 - length * math.sin(rad) 
        self.canvas.coords(self.pointer, 100, 100, x, y)
        
        # Update LED color
        if buzzer_state == 1:
            self.led_canvas.itemconfig(self.led, fill="red")
        else:
            self.led_canvas.itemconfig(self.led, fill="gray")

    def log_to_csv(self, time_ms, angle, buzzer_state):
        """Appends a single row of data to the CSV."""
        try:
            with open(CSV_FILENAME, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([time_ms, angle, buzzer_state])
        except Exception as e:
            pass 

# Run the application
if __name__ == "__main__":
    root = tk.Tk()
    app = ArduinoMonitorApp(root)
    root.mainloop()