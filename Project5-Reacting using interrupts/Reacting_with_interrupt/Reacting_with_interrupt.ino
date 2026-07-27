int Button_PIN=6;
int LED_PIN=4;
int Int_PIN=2;
volatile bool buttonPressed = false;

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(Button_PIN, INPUT);
  pinMode(Int_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(Int_PIN), handleButtonPress, CHANGE);

  Serial.println("System Initialized. Ready for button presses.");
}

void loop() {
  for (int i = 0; i< 10000; i++){
    Serial.println("calculating...");
}
}

void handleButtonPress() {
  buttonPressed = digitalRead(Int_PIN);
  if (buttonPressed) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}