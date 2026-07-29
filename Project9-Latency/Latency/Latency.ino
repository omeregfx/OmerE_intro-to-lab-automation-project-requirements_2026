// Define the hardware pins
const int buttonPin = 6;  // Connect your button here
const int ledPin = 4;    // Using the built-in LED (or wire an external one to pin 13)

unsigned long startTime;
unsigned long latency;

void setup() {
  // Initialize serial communication at 9600 baud to match the Python script
  Serial.begin(9600);
  
  // Configure the pins
  pinMode(ledPin, OUTPUT);
  // Using INPUT_PULLUP simplifies wiring (no external resistor needed)
  // The button should connect Pin 2 to Ground.
  pinMode(buttonPin, INPUT);
  
  // Seed the random number generator using an unconnected analog pin
  randomSeed(analogRead(0));
}

void loop() {
  // 1. Turn the LED on to signal the start of the trial
  digitalWrite(ledPin, HIGH);
  
  // 2. Wait a random amount of time (e.g., between 2 and 6 seconds) 
  // so the user cannot anticipate when the light will turn off.
  delay(random(2000, 6000));
  
  // 3. Turn the LED off and immediately record the start time
  digitalWrite(ledPin, LOW);
  startTime = millis();
  
  // 4. Wait for the user to press the button. 
  // Because we use INPUT_PULLUP, a pressed button reads as LOW.
  while (digitalRead(buttonPin) == HIGH) {
    // Do nothing, just wait for the reaction
  }
  
  // 5. Calculate the latency (reaction time)
  latency = millis() - startTime;
  
  // 6. Send the latency to the Python script over the Serial port.
  // println adds the newline character that Python's readline() is looking for.
  Serial.println(latency);
  
  // 7. Wait a few seconds before starting the next round
  delay(3000);
}