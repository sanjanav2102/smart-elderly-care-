#define MQ2_ANA A1
#define MQ2_DIG 2
#define TEMP_SENSOR A0

const int ledCount = 10;    // Number of LEDs in the bar graph
int ledPins[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12}; // Pins for LEDs of bar graph

void setup() {
  Serial.begin(9600);
  pinMode(MQ2_ANA, INPUT);
  pinMode(MQ2_DIG, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize LED pins as OUTPUT
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // Read gas sensor values
  float gas_analog = analogRead(MQ2_ANA);
  int gas_digital = digitalRead(MQ2_DIG);

  if (gas_digital == 1){
    Serial.println("Gas detected.");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    delay(10);                      
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off
    delay(10); 
  }

  // Read temperature
  float temperature = analogRead(TEMP_SENSOR);
  if (temperature < 300){
    Serial.println("Evacuate the room !");
  }
  else if (temperature < 450){
    Serial.println("Temperature approaching dangerous levels...");
  }

  // Map temperature to LED levels
  int ledLevel = map(temperature, 115, 953, 0, ledCount); 

  // Update LED bar graph
  for (int i = 0; i < ledCount; i++) {
    if (i < ledLevel) {
      digitalWrite(ledPins[i], LOW); // Turn off LED
    } else {
      digitalWrite(ledPins[i], HIGH);  // Turn on LED
    }
  }

  delay(500);
}