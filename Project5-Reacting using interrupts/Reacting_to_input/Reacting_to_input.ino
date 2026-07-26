int Button_PIN=6;
int LED_PIN=4;

void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(Button_PIN, INPUT);

  Serial.println("System Initialized. Ready for button presses.");
}

void loop() {
  // put your main code here, to run repeatedly:
  int Button_STATE = digitalRead(Button_PIN);
  
  for (int i = 0; i< 10000; i++){
    Serial.println("calculating...");
}

  if (Button_STATE == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Button pressed, LED is ON");
  }
  else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Button released, LED is OFF");
  }
}
