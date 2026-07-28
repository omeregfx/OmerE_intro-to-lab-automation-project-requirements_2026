#include <MD_PWM.h>

int LED_PIN = 4;
int KNOB_PIN = A0;

MD_PWM ledPWM(LED_PIN);

void setup() {
  ledPWM.begin(50);
  pinMode(KNOB_PIN, INPUT); 
}

void loop() {
  int knobValue = analogRead(KNOB_PIN);
  int pwmValue = map(knobValue, 0, 1023, 0, 255);
  ledPWM.write(pwmValue);
}