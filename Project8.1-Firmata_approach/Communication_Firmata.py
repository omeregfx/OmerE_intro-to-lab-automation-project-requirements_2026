import FreeSimpleGUI as sg
from telemetrix import telemetrix
import threading
import time

# --- Hardware Configuration ---
COM_PORT = "COM4" 
LED_PIN = 4
BUTTON_PIN = 6
INTERRUPT_PIN = 2

def main():
    # --- 1. GUI Layout Definition ---
    sg.theme('LightBlue')
    layout = [
        [sg.Text('Telemetrix Arduino Controller', font=('Helvetica', 14, 'bold'))],
        [sg.Text('Button State:', size=(15, 1)), sg.Text('Waiting...', key='-BTN_STATE-', text_color='blue')],
        [sg.Text('LED State:', size=(15, 1)), sg.Text('OFF', key='-LED_STATE-', text_color='red')],
        [sg.Text('LED Duration (ms):', size=(15, 1)), sg.InputText('30', key='-DURATION-', size=(10, 1))],
        [sg.Text('System Logs:')],
        [sg.Multiline(size=(45, 10), key='-LOG-', disabled=True, autoscroll=True)],
        [sg.Button('Exit')]
    ]

    window = sg.Window('Arduino GUI Timer', layout, finalize=True)

    # --- 2. Initialize Telemetrix ---
    try:
        window['-LOG-'].update(f"Connecting to Arduino on {COM_PORT}...\n")
        # Initialize the board using telemetrix
        board = telemetrix.Telemetrix(com_port=COM_PORT)
        window['-LOG-'].update("Connected successfully.\n", append=True)
    except Exception as e:
        sg.popup_error(f"Failed to connect to Arduino: {e}\nMake sure Telemetrix4Arduino is uploaded and the port is free.")
        window.close()
        return

    led_timer = None

    # --- 3. Hardware Callbacks & Functions ---
    def turn_off_led():
        """
        Executed by the threading.Timer when the interval expires.
        Sends a command to turn off the LED and securely updates the GUI.
        """
        board.digital_write(LED_PIN, 0)
        window.write_event_value('-UPDATE_STATE-', ('LED', 'OFF', 'red'))
        window.write_event_value('-LOG_MSG-', 'Timer finished. LED turned OFF.')

    def button_callback(data):
        """
        Executed automatically by Telemetrix in a background thread when Pin 2 changes.
        data format: [pin_type, pin_number, pin_value, time_stamp]
        """
        pin_value = data[2]
        
        # 1 (HIGH)) means the button is pressed (assuming pull-up logic)
        if pin_value == 1:
            window.write_event_value('-UPDATE_STATE-', ('BTN', 'PRESSED', 'green'))
            window.write_event_value('-TRIGGER_LED-', None)
        else:
            window.write_event_value('-UPDATE_STATE-', ('BTN', 'RELEASED', 'blue'))

    # --- 4. Pin Configuration ---
    
    # Configure LED Pin
    board.set_pin_mode_digital_output(LED_PIN)
    board.digital_write(LED_PIN, 0) 
    
    # Configure Button Pin (Pin 6) with an internal pull-up resistor
    board.set_pin_mode_digital_input_pullup(BUTTON_PIN)
    
    # Configure Interrupt Pin (Pin 2) as standard input and attach the callback.
    # This reads the state changes coming from Pin 6 via your physical wire connection.
    board.set_pin_mode_digital_input(INTERRUPT_PIN, callback=button_callback)

    # --- 5. Main GUI Event Loop ---
    while True:
        event, values = window.read()

        if event in (sg.WIN_CLOSED, 'Exit'):
            break

        if event == '-UPDATE_STATE-':
            target, state_text, color = values['-UPDATE_STATE-']
            if target == 'BTN':
                window['-BTN_STATE-'].update(state_text, text_color=color)
            elif target == 'LED':
                window['-LED_STATE-'].update(state_text, text_color=color)

        if event == '-LOG_MSG-':
            window['-LOG-'].update(values['-LOG_MSG-'] + '\n', append=True)

        if event == '-TRIGGER_LED-':
            # Turn on the LED immediately
            board.digital_write(LED_PIN, 1)
            window['-LED_STATE-'].update('ON', text_color='green')
            window['-LOG-'].update('Button Pressed -> LED ON.\n', append=True)
            
            # Parse the time duration
            try:
                duration_ms = float(values['-DURATION-'])
                duration_s = duration_ms / 1000.0 
            except ValueError:
                duration_s = 0.03 # Default to 30ms
                window['-LOG-'].update('! Invalid time format. Defaulting to 30ms.\n', append=True)
                
            # Cancel existing timer if running, then start a new one
            if led_timer and led_timer.is_alive():
                led_timer.cancel()
            
            led_timer = threading.Timer(duration_s, turn_off_led)
            led_timer.start()

    # --- 6. Cleanup ---
    if led_timer and led_timer.is_alive():
        led_timer.cancel()
    board.shutdown() 
    window.close()

if __name__ == '__main__':
    main()