import FreeSimpleGUI as sg
import serial
import threading
import time

# --- Configuration ---
# CHANGE THIS to the port your device is connected to
SERIAL_PORT = 'COM4' 
BAUD_RATE = 9600

def serial_read_thread(ser, window):
    """
    Background thread to continuously read from the serial port.
    It passes the received data safely to the FreeSimpleGUI main loop using write_event_value.
    """
    while True:
        try:
            if ser.in_waiting > 0:
                # Read line, decode to string, and remove trailing whitespace/newlines
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    # Safely trigger an event in the main GUI thread
                    window.write_event_value('-SERIAL_DATA-', line)
        except serial.SerialException as e:
            window.write_event_value('-SERIAL_ERROR-', f"Connection lost: {e}")
            break
        except Exception as e:
            window.write_event_value('-SERIAL_ERROR-', str(e))
            break
        
        # Short sleep to prevent the thread from maxing out the CPU
        time.sleep(0.01)

def main():
    # --- GUI Layout Definition ---
    layout = [
        [sg.Text(f'Connecting to {SERIAL_PORT}...', key='-STATUS-', text_color='yellow')],
        [sg.Text('LED Duration (ms):'), sg.Input(key='-DURATION-', size=(10, 1)), sg.Button('Send')],
        [sg.Text('Device Responses:')],
        [sg.Multiline(size=(40, 15), key='-OUTPUT-', disabled=True, autoscroll=True)],
        [sg.Button('Exit')]
    ]

    window = sg.Window('Arduino Serial Controller', layout, finalize=True)

    # --- Serial Port Initialization ---
    ser = None
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        window['-STATUS-'].update(f'Connected to {SERIAL_PORT}', text_color='green')
        
        # Start the background thread for parallel serial reading
        # daemon=True ensures the thread dies when the main program closes
        threading.Thread(target=serial_read_thread, args=(ser, window), daemon=True).start()
    except serial.SerialException as e:
        window['-STATUS-'].update('Connection Failed', text_color='red')
        sg.popup_error(f"Failed to connect to {SERIAL_PORT}.\nCheck if the port is correct and not in use by another program.\n\nError details: {e}")

    # --- Main GUI Event Loop ---
    while True:
        event, values = window.read()

        # Handle window closure or Exit button
        if event in (sg.WIN_CLOSED, 'Exit'):
            break

        # Handle Send button click
        if event == 'Send':
            if ser and ser.is_open:
                duration = values['-DURATION-']
                # Validate input is a positive integer
                if duration.isdigit():
                    try:
                        # Append newline and encode to bytes before sending
                        data_to_send = f"{duration}\n".encode('utf-8')
                        ser.write(data_to_send)
                        window['-OUTPUT-'].update(f"-> Sent duration: {duration} ms\n", append=True)
                    except Exception as e:
                        window['-OUTPUT-'].update(f"! Error sending data: {e}\n", append=True)
                else:
                    sg.popup_error("Please enter a valid positive integer for the milliseconds.")
            else:
                sg.popup_error("Cannot send data. Serial port is not connected.")

        # Handle incoming data routed from the background thread
        if event == '-SERIAL_DATA-':
            msg = values['-SERIAL_DATA-']
            
            # Map the numerical codes to descriptive state messages
            if msg == '0':
                display_msg = "Device: '0' - LED off"
            elif msg == '1':
                display_msg = "Device: '1' - Button and LED on"
            elif msg == '2':
                display_msg = "Device: '2' - Button off"
            else:
                display_msg = f"Device: Unknown message received -> '{msg}'"
            
            # Append the message to the multiline output box
            window['-OUTPUT-'].update(f"<- {display_msg}\n", append=True)
        
        # Handle exceptions thrown by the serial reading thread
        if event == '-SERIAL_ERROR-':
            error_msg = values['-SERIAL_ERROR-']
            window['-OUTPUT-'].update(f"! {error_msg}\n", append=True)
            window['-STATUS-'].update('Disconnected', text_color='red')

    # --- Cleanup ---
    if ser and ser.is_open:
        ser.close()
    window.close()

if __name__ == '__main__':
    main()