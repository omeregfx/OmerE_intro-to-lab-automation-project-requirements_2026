#include <Servo.h>

Servo motor;
int Knob_PIN = 0;
int Motor_PIN = 7;
int MovementThreshold = 3;
int LastAngle = -1;

void setup() {
  // put your setup code here, to run once:
  motor.attach(Motor_PIN);
  pinMode(Knob_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int RawRead = analogRead(Knob_PIN);
  int CurrentAngle = map(RawRead, 0, 1023, 0, 168);

  if (abs(CurrentAngle - LastAngle) >= MovementThreshold) {
    motor.write(CurrentAngle);
    LastAngle = CurrentAngle;
  }  
}
