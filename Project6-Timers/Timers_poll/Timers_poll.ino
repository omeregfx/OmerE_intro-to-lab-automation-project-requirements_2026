int Button_PIN = 6;
int LED_PIN = 4;
int Int_PIN = 2;

volatile bool buttonPressed = false;
volatile bool LEDactive = false;
volatile unsigned long startTime = 0; 
unsigned long Time;

const unsigned long interval = 5000;

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(Button_PIN, INPUT);
  pinMode(Int_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(Int_PIN), handleButtonPress, CHANGE);
}

void loop() {
  // Time = millis();
  // delay(1000);
  // Serial.println(Time / 1000);
  if (LEDactive && (millis() - startTime >= interval)) {
    digitalWrite(LED_PIN, LOW);
    LEDactive = false;
  }

  for (int i = 0; i< 1000; i++){
    delay(10);
  }
}

void handleButtonPress() {
  buttonPressed = digitalRead(Int_PIN);

  if (buttonPressed) {
    digitalWrite(LED_PIN, HIGH);
    startTime = millis(); 
    LEDactive = true;
  } 
}