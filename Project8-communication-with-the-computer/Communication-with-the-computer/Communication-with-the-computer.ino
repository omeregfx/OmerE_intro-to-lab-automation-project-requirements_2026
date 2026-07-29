#include <MsTimer2.h>

// --- Pin Definitions ---
int LED_PIN = 4;
int Int_PIN = 2; // Button must be connected to Pin 2 for the interrupt

// --- Global Variables ---
volatile bool buttonStateChanged = false; 
volatile bool ledActive = false;    
unsigned long ledONTime = 1000;     

// --- Forward Declarations ---
void handleButtonPress();
void turn_off();

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Use INPUT_PULLUP to use the Arduino's internal resistor
  // The button should connect Pin 2 to Ground.
  pinMode(Int_PIN, INPUT); 

  // Trigger interrupt whenever the button state changes
  attachInterrupt(digitalPinToInterrupt(Int_PIN), handleButtonPress, CHANGE);
}

void loop() {
  // Monitor Serial Port for incoming user inputs
  if (Serial.available() > 0) {
    long recievedTime = Serial.parseInt();

    while (Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    if (recievedTime > 0) {
      ledONTime = (unsigned long)recievedTime;
      // Removed the "I recieved:" print to keep serial comms clean for the GUI
    }
  }

  // Handle Button Press Event
  if (buttonStateChanged) {
    buttonStateChanged = false; // Reset interrupt flag
    
    // Read the actual state of the pin
    // With INPUT_PULLUP, LOW means the button is pressed down
    int currentState = digitalRead(Int_PIN);

    if (currentState == HIGH) { // Button is pressed
      if (!ledActive) {
        ledActive = true;
        digitalWrite(LED_PIN, HIGH); // Light up LED
        Serial.println("1");         // State 1: Button and LED on

        unsigned long timerDuration = ledONTime + 1;
        MsTimer2::set(timerDuration, turn_off);
        MsTimer2::start();
      }
    } else { // Button is released
      Serial.println("2"); // State 2: Button off
    }
  }
}

// ISR: Triggered on any state change on Pin 2
void handleButtonPress() {
  buttonStateChanged = true;
}

// Timer Callback: Executed automatically when MsTimer2 expires.
void turn_off() {
  digitalWrite(LED_PIN, LOW);
  ledActive = false;
  Serial.println("0"); // State 0: LED off
  MsTimer2::stop();
}