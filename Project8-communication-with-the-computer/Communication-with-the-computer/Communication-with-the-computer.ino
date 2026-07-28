#include <MsTimer2.h>

// --- Pin Definitions ---
int Button_PIN=6;
int LED_PIN=4;
int Int_PIN=2;

// --- Global Variables ---
volatile bool buttonPressed = false; // Flag set by ISR when button state changes
volatile bool ledActive = false;    // Tracks if LED timer is currently running
unsigned long ledONTime = 1000;     // Default ON time: 1000 ms (until updated via Serial)

// --- Forward Declarations ---
void handleButtonPress();
void turn_off();

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);

  // Configure hardware pin modes
  pinMode(LED_PIN, OUTPUT);
  // Ensure LED is initially OFF
  digitalWrite(LED_PIN, LOW);
  pinMode(Button_PIN, INPUT);
  pinMode(Int_PIN, INPUT);

  // Attach hardware interrupt to detect button press
  attachInterrupt(digitalPinToInterrupt(Int_PIN), handleButtonPress, CHANGE);
}

void loop() {
  // Monitor Serial Port for incoming user inputs
  if (Serial.available() > 0) {
    // Read integer of variable length from serial input stream
    long recievedTime = Serial.parseInt();

    // Clear remaining trailing characters (e.g., newline '\n' or carriage return '\r')
    while (Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    // Error handling and input validation
    if (recievedTime > 0) {
      ledONTime = (unsigned long)recievedTime;

      // Print confirmation message back over Serial
      Serial.print("I recieved:");
      Serial.println(ledONTime);
    } else if (recievedTime < 0) {
      Serial.println("Error: Please send a positive integer for time in ms.");
    }
  }

  // Handle Button Press Event
  if (buttonPressed) {
    buttonPressed = false; // Reset interrupt flag

    // Only start if the LED is not already on
    if (!ledActive) {
      ledActive = true;
      digitalWrite(LED_PIN, HIGH); // Light up LED

      // Account for the MsTimer2 1 ms offset bug observed in Project 6
      unsigned long timerDuration = ledONTime + 1;

      // Configure and start hardware timer
      MsTimer2::set(timerDuration, turn_off);
      MsTimer2::start();
    }
  }
}

// ISRs

// ISR: Triggered on button press (FALLING edge on Pin 2)
// Sets the volatile flag to be handled safely inside the main loop.
void handleButtonPress() {
  buttonPressed = true;
}

// Timer Callback: Executed automatically when MsTimer2 expires.
// Turns off the LED and stops the timer instance.
void turn_off() {
  digitalWrite(LED_PIN, LOW);
  ledActive = false;
  MsTimer2::stop();
}