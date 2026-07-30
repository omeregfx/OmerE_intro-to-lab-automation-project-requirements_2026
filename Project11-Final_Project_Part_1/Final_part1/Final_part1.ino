/*
 * Fan & Servo Controller System with Button Toggle
 * Reads accelerometer Y-axis data to control a servo-mounted fan.
 * A push button on Pin 6 toggles the fan ON and OFF.
 * Triggers a buzzer and forces the fan off if the angle exceeds a safe threshold.
 * Outputs status via OLED display and Serial port.
 */

#include "Arduino_SensorKit.h"
#include <Servo.h>

// --- Hardware Constants ---
const int SERVO_PIN = 3;
const int FAN_PIN = 7;
const int BUZZER_PIN = 5; 
const int BUTTON_PIN = 6; // Push button pin

// --- Logic Constants ---
const int HIGH_ANGLE_THRESHOLD = 140; // High threshold in degrees to trigger the alarm
const int LOW_ANGLE_THRESHOLD = 20;   // Low threshold in degrees to trigger the alarm
const int LOOP_DELAY_MS = 0;        // Main loop delay
const float MOVEMENT_THRESHOLD = 0.1; // Max allowed change in yVal per loop (spike filter)

// --- Global Variables ---
Servo fanServo;
int targetAngle = 0;  // Where the sensor wants the servo to go
int currentAngle = 0; // Where the servo currently is physically located
bool isBuzzerActive = false;
float yVal = 0.0;
float lastYVal = 0.0; // Tracks the previous reading to calculate the change

// Button State Variables
bool isFanPowerOn = false;     
int lastButtonState = LOW;    

// --- NEW: Non-blocking Timer Variables ---
unsigned long lastReportTime = 0;
const int REPORT_INTERVAL_MS = 50; // How often to send data/update screen

void setup() {
  // 1. Initialize Communications
  Serial.begin(9600);
  
  // 2. Initialize Hardware Pins
  fanServo.attach(SERVO_PIN);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  // 3. Initialize SensorKit Modules
  Oled.begin();
  Oled.setFlipMode(true);
  Accelerometer.begin();
}

void loop() {
  // 1. These run as fast as possible for instantly responsive motor control
  readButtonInput();
  calculateAngle();
  updateHardwareState();
  
  // 2. These only run every 50ms so we don't overwhelm the PC or the OLED
  if (millis() - lastReportTime >= REPORT_INTERVAL_MS) {
    updateOLEDDisplay();
    sendDataToPython();
    lastReportTime = millis();
  }
  
  // We keep the main delay at 0!
  delay(LOOP_DELAY_MS); 
}

/*
 * Checks for a button press and toggles the fan power state
 */
void readButtonInput() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  // Detect when the button transitions from unpressed (LOW) to pressed (HIGH)
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    isFanPowerOn = !isFanPowerOn; // Toggle the state (ON -> OFF or OFF -> ON)
  }

  lastButtonState = currentButtonState; // Save current state for next loop
}

/* 
 * Reads the accelerometer, applies a deadzone and noise filter, 
 * and calculates the target angle 
 */
void calculateAngle() { 
  // 1. Get the raw new reading
  float newYVal = Accelerometer.readY();

  // -- DEADZONE FILTER --
  // If the reading is very close to 0 (between -0.15 and 0.15), force it perfectly flat
  if (abs(newYVal) < 0.1) {
    newYVal = 0.0;
  }

  // Round to 1 decimal place
  newYVal = round(newYVal * 10.0) / 10.0; 

  // 2. Check if the change is WITHIN the allowed threshold (using your updated < sign)
  if (abs(newYVal - lastYVal) < MOVEMENT_THRESHOLD) {
    // It's a smooth movement, so we accept the new value and set the target
    yVal = newYVal;
    targetAngle = map(yVal * 100, -100, 100, 0, 165);
  }

  // 3. Always update lastYVal so the system knows where the sensor actually is
  lastYVal = newYVal;
}

/*
 * Controls the Fan, Servo, and Buzzer based on angle and button state
 */
void updateHardwareState() {
  
  // Smoothly move the servo to the target angle in 1-degree steps
  while (currentAngle != targetAngle) {
    if (currentAngle < targetAngle) {
      currentAngle++;
    } else {
      currentAngle--;
    }
    
    fanServo.write(currentAngle);
    delay(10); // 10ms delay gives the physical motor time to move 1 degree smoothly
  }
  
  // Safety check against threshold using the current physical angle
  if (currentAngle > HIGH_ANGLE_THRESHOLD || currentAngle < LOW_ANGLE_THRESHOLD) {
    // Exceeded threshold: Force fan OFF and sound alarm
    isBuzzerActive = true;
    digitalWrite(FAN_PIN, LOW);
    tone(BUZZER_PIN, 1000);
  } else {
    // Normal operation: Stop buzzer, set fan based on user toggle state
    isBuzzerActive = false;
    noTone(BUZZER_PIN);

    if (isFanPowerOn) {
      digitalWrite(FAN_PIN, HIGH);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }
  }
}

/*
 * Updates the local OLED screen with Angle, Alarm state, and Fan state
 */
void updateOLEDDisplay() {
  Oled.setFont(u8x8_font_chroma48medium8_r);
  
  // Row 0: Angle
  Oled.setCursor(0, 0);
  Oled.print("Angle: ");
  Oled.print(currentAngle);
  Oled.print(" deg  "); 

  // Row 2: Alarm
  Oled.setCursor(0, 2);
  Oled.print("Alarm: ");
  if (isBuzzerActive) {
    Oled.print("ON ");
  } else {
    Oled.print("OFF");
  }

  // Row 4: Fan Power State
  Oled.setCursor(0, 4);
  Oled.print("Fan  : ");
  if (isFanPowerOn && !isBuzzerActive) {
    Oled.print("ON ");
  } else {
    Oled.print("OFF");
  }
  
  Oled.refreshDisplay();
}

/*
 * Sends a comma-separated string to the PC: Time(ms),yVal,Angle,BuzzerState
 */
void sendDataToPython() {
  unsigned long currentTime = millis();
  
  Serial.print(currentTime);
  Serial.print(",");
  Serial.print(yVal);
  Serial.print(",");
  Serial.print(currentAngle);
  Serial.print(",");
  Serial.println(isBuzzerActive ? 1 : 0);
}