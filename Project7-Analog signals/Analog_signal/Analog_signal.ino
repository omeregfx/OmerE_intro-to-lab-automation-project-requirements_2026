#include <MD_PWM.h>

const uint8_t LED_PIN = 4;
const uint8_t KNOB_PIN = A0;

// Create an MD_PWM object for Pin 4
MD_PWM ledPWM(LED_PIN);

void setup() {
  ledPWM.begin();

  pinMode(KNOB_PIN, INPUT); 
}

void loop() {
  int knobValue = analogRead(KNOB_PIN);
  int pwmValue = map(knobValue, 0, 1023, 0, 255);
  ledPWM.write(pwmValue);
  delay(10);
}