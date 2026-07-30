/*
 * Fan & Servo Controller System with Button Toggle
 * Reads accelerometer Y-axis data to control a servo-mounted fan.
 * Uses non-blocking timers and an anti-jitter deadband for smooth servo control.
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
const int HIGH_ANGLE_THRESHOLD = 140; 
const int LOW_ANGLE_THRESHOLD = 20;   
const int LOOP_DELAY_MS = 0;        
const int JITTER_DEADBAND = 5; // Minimum degrees the target must change before servo moves

// --- Global Variables ---
Servo fanServo;
int targetAngle = 0;  // Where the sensor wants the servo to go
int currentAngle = 0; // Where the servo currently is physically located
bool isBuzzerActive = false;
float yVal = 0.0;

// Button State Variables
bool isFanPowerOn = false;     
int lastButtonState = LOW;    

// --- Non-blocking Timer Variables ---
unsigned long lastReportTime = 0;
const int REPORT_INTERVAL_MS = 50; // How often to send data to the PC and OLED

void setup() {
  Serial.begin(9600);
  
  fanServo.attach(SERVO_PIN);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  Oled.begin();
  Oled.setFlipMode(true);
  Accelerometer.begin();
}

void loop() {
  readButtonInput();
  calculateAngle();
  updateHardwareState();
  
  // Update screens and PC every 50ms without freezing the motor
  if (millis() - lastReportTime >= REPORT_INTERVAL_MS) {
    updateOLEDDisplay();
    sendDataToPython();
    lastReportTime = millis();
  }
  
  delay(LOOP_DELAY_MS); 
}

/*
 * Checks for a button press and toggles the fan power state
 */
void readButtonInput() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == HIGH && lastButtonState == LOW) {
    isFanPowerOn = !isFanPowerOn; 
  }
  lastButtonState = currentButtonState; 
}

/* 
 * Reads the accelerometer and applies an anti-jitter deadband 
 */
void calculateAngle() { 
  // 1. Get the precise raw reading (No rounding to 1 decimal place anymore!)
  float newYVal = Accelerometer.readY();

  // 2. Flat deadzone: force perfectly flat if very close to 0
  if (abs(newYVal) <= 0.15) {
    newYVal = 0.0;
  }

  // 3. Map the precise value directly to a potential target angle
  int potentialAngle = map(newYVal * 100, -100, 100, 0, 165);

  // 4. ANTI-JITTER DEADBAND
  // Only change the target if the difference is greater than our 3-degree threshold
  if (abs(potentialAngle - targetAngle) >= JITTER_DEADBAND) {
    targetAngle = potentialAngle;
    yVal = newYVal; // Update the sensor value sent to Python
  }
}

/*
 * Controls the Fan, Servo, and Buzzer based on angle and button state
 */
void updateHardwareState() {
  
  // Smoothly move the servo 1 degree at a time without freezing the main loop
  if (currentAngle != targetAngle) {
    if (currentAngle < targetAngle) {
      currentAngle++;
    } else {
      currentAngle--;
    }
    
    fanServo.write(currentAngle);
    delay(10); // 10ms delay for smooth gears
  }
  
  // Safety check against threshold using the physical angle
  if (currentAngle > HIGH_ANGLE_THRESHOLD || currentAngle < LOW_ANGLE_THRESHOLD) {
    isBuzzerActive = true;
    digitalWrite(FAN_PIN, LOW);
    tone(BUZZER_PIN, 1000);
  } else {
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
  
  Oled.setCursor(0, 0);
  Oled.print("Angle: ");
  Oled.print(currentAngle);
  Oled.print(" deg  "); 

  Oled.setCursor(0, 2);
  Oled.print("Alarm: ");
  if (isBuzzerActive) {
    Oled.print("ON ");
  } else {
    Oled.print("OFF");
  }

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