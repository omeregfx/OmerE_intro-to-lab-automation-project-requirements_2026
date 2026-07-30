/*
 * Fan & Servo Controller System with Button Toggle
 * Reads accelerometer X-axis data to control a servo-mounted fan.
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
const int LOW_ANGLE_THRESHOLD = 20; // Low threshold in degrees to trigger the alarm
const int LOOP_DELAY_MS = 200;   // Update rate for smooth servo movement

// --- Global Variables ---
Servo fanServo;
int currentAngle = 0;
bool isBuzzerActive = false;

// Button State Variables
bool isFanPowerOn = true;     // Tracks whether user turned fan ON or OFF via button
int lastButtonState = LOW;    // Remembers previous button state for edge detection

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
  readButtonInput();
  calculateAngle();
  updateHardwareState();
  updateOLEDDisplay();
  sendDataToPython();
  
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
 * Reads the accelerometer and maps the X-axis to a 0-165 degree angle 
 */
void calculateAngle() {
  float yVal = Accelerometer.readY();
  
  // Multiply by 100 to use integer mapping
  currentAngle = map(yVal * 8, -1, 1, 0, 165);
  currentAngle = constrain(currentAngle, 0, 165); 
}

/*
 * Controls the Fan, Servo, and Buzzer based on angle and button state
 */
void updateHardwareState() {
  // Move the servo to the new angle
  fanServo.write(currentAngle);
  
  // Safety check against threshold
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
 * Sends a comma-separated string to the PC: Time(ms),Angle,BuzzerState
 */
void sendDataToPython() {
  unsigned long currentTime = millis();
  
  Serial.print(currentTime);
  Serial.print(",");
  Serial.print(currentAngle);
  Serial.print(",");
  Serial.println(isBuzzerActive ? 1 : 0);
}