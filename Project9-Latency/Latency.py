import threading
import queue
import serial
import csv
import os
from datetime import datetime
import PySimpleGUI as sg
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# ----------------------------------------------------------------------
# 1. Matplotlib Helper Function
# ----------------------------------------------------------------------
def draw_figure(canvas, figure):
    """
    Embeds a matplotlib figure into a PySimpleGUI Canvas element.
    """
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

# ----------------------------------------------------------------------
# 2. Serial Reading Function (Runs in a separate thread)
# ----------------------------------------------------------------------
def read_serial(com_port, baud_rate, data_queue, stop_event):
    """
    Continuously reads latency data from the serial port.
    Runs in parallel with the main GUI loop.
    """
    try:
        with serial.Serial(com_port, baud_rate, timeout=1) as ser:
            print(f"Connected to {com_port} at {baud_rate} baud.")
            while not stop_event.is_set():
                try:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        latency = float(line)
                        data_queue.put(latency)
                except ValueError:
                    print(f"Received malformed data: {line}")
                except serial.SerialException as e:
                    print(f"Serial communication error: {e}")
                    break
    except serial.SerialException as e:
        print(f"Failed to connect to {com_port}: {e}")
        data_queue.put(f"ERROR: {e}")

# ----------------------------------------------------------------------
# 3. Main GUI Application
# ----------------------------------------------------------------------
def main():
    sg.theme('LightGrey1')

    # Updated Layout: Changed button to 'Save'
    layout = [
        [
            sg.Text('COM Port Number (e.g., 3 for COM3):'),
            sg.Input(key='-PORT-', size=(5, 1), default_text='3'),
            sg.Button('Connect', key='-CONNECT-'),
            sg.Button('Save', key='-SAVE-'),  # UPDATED BUTTON
            sg.Text('', key='-STATUS-', text_color='blue')
        ],
        [sg.Canvas(key='-CANVAS-', size=(500, 400))]
    ]

    window = sg.Window('Real-Time Latency Histogram', layout, finalize=True)

    fig, ax = plt.subplots(figsize=(5, 4), dpi=100)
    ax.set_title('Reaction Time Latency')
    ax.set_xlabel('Latency (ms)')
    ax.set_ylabel('Frequency')
    fig_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)

    data_queue = queue.Queue()
    stop_event = threading.Event()
    serial_thread = None
    latencies = []

    # ------------------------------------------------------------------
    # 4. Main Event Loop
    # ------------------------------------------------------------------
    while True:
        event, values = window.read(timeout=100)

        if event == sg.WIN_CLOSED:
            break

        # Handle Connect Button
        if event == '-CONNECT-':
            port_num = values['-PORT-']
            com_port = f"COM{port_num}"
            baud_rate = 9600
            if serial_thread is None or not serial_thread.is_alive():
                stop_event.clear()
                latencies.clear()
                serial_thread = threading.Thread(
                    target=read_serial,
                    args=(com_port, baud_rate, data_queue, stop_event),
                    daemon=True
                )
                serial_thread.start()
                window['-STATUS-'].update(f'Connecting to {com_port}...')
                window['-CONNECT-'].update(disabled=True)

        # Handle Save Button
        if event == '-SAVE-':
            if not latencies:
                sg.popup_error('No latency data available to save!', title='Error')
            else:
                # Open a folder dialog to choose the directory
                save_dir = sg.popup_get_folder('Select Directory to Save Files', no_window=True)
                
                if save_dir:
                    try:
                        # Generate a timestamp to create unique filenames
                        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                        csv_path = os.path.join(save_dir, f'latency_data_{timestamp}.csv')
                        png_path = os.path.join(save_dir, f'latency_histogram_{timestamp}.png')
                        
                        # 1. Save CSV
                        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
                            writer = csv.writer(f)
                            writer.writerow(['Trial Number', 'Latency (ms)'])
                            for idx, val in enumerate(latencies, 1):
                                writer.writerow([idx, val])
                        
                        # 2. Save Histogram PNG
                        fig.savefig(png_path)
                        
                        sg.popup(f'Files successfully saved to:\n{save_dir}', title='Success')
                    except Exception as e:
                        sg.popup_error(f'Failed to save files:\n{e}', title='Save Error')

        # Process new data from the queue
        new_data_received = False
        while not data_queue.empty():
            item = data_queue.get()
            if isinstance(item, str) and item.startswith("ERROR"):
                window['-STATUS-'].update(item, text_color='red')
                window['-CONNECT-'].update(disabled=False)
            else:
                latencies.append(item)
                new_data_received = True
                window['-STATUS-'].update('Receiving data...', text_color='green')

        # Update the histogram
        if new_data_received and len(latencies) > 0:
            ax.clear()
            ax.set_title('Reaction Time Latency')
            ax.set_xlabel('Latency (ms)')
            ax.set_ylabel('Frequency')
            ax.hist(latencies, bins=10, color='skyblue', edgecolor='black')
            fig_agg.draw()

    # ------------------------------------------------------------------
    # 5. Cleanup upon exit
    # ------------------------------------------------------------------
    stop_event.set()
    if serial_thread is not None:
        serial_thread.join(timeout=1)
    window.close()

if __name__ == '__main__':
    main()