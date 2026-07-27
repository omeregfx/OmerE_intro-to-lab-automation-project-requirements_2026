#include <MsTimer2.h>

int Button_PIN=6;
int LED_PIN=4;
int Int_PIN=2;
volatile bool buttonPressed = false;
unsigned long Time;

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(Button_PIN, INPUT);
  pinMode(Int_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(Int_PIN), handleButtonPress, CHANGE);
}

void loop() {
  // Time = millis();
  // Serial.println(Time / 1000);
  // delay(1000);
  // for (int i = 0; i< 10000; i++){
  //   Serial.println("calculating...");
  //   delay(1000);
  // }
}

void handleButtonPress() {
  buttonPressed = digitalRead(Int_PIN);

  if (buttonPressed) {
    digitalWrite(LED_PIN, HIGH);
    MsTimer2::set(30, turnoffLED);
    MsTimer2::start();
  } 
}

void turnoffLED() {
  digitalWrite(LED_PIN, LOW);
  buttonPressed = false;
  MsTimer2::stop();
}