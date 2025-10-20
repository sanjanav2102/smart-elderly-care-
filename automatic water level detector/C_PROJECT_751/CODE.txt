#include <EEPROM.h>
#include <LiquidCrystal.h>

// Initialize LCD with the specified pin configuration
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// Structure to hold water level data
struct WaterLevelData {
  long inches;        
  int set_val;        
  int percentage;     
};
// Create an instance of the structure
WaterLevelData waterLevel; 
// Pointer to the waterLevel structure            
WaterLevelData *waterLevelPtr = &waterLevel; 

long duration;                         
bool state, pump;                      

void setup() {
  lcd.begin(16, 2);                    
  lcd.print("WATER LEVEL: ");
  lcd.setCursor(0, 1);
  lcd.print("PUMP:OFF MANUAL");

  // Set pin modes for pins 8 to 12
  for (int pin = 8; pin <= 12; ++pin) {
    if (pin == 8 || pin == 12) {
      pinMode(pin, OUTPUT);            
    } else {
      pinMode(pin, INPUT_PULLUP);      
    }
  }

  waterLevelPtr->set_val = EEPROM.read(0); 
  if (waterLevelPtr->set_val > 150) {
    waterLevelPtr->set_val = 150;      
  }
}

void loop() {
  digitalWrite(3, LOW);                
  delayMicroseconds(2);
  digitalWrite(8, HIGH);               
  delayMicroseconds(10);
  digitalWrite(8, LOW);
  
  duration = pulseIn(9, HIGH);         
  
  // Calculate and display inches and percentage values using pointers
  waterLevelPtr->inches = microsecondsToInches(duration); 
  waterLevelPtr->percentage = (waterLevelPtr->set_val - waterLevelPtr->inches) * 100 / waterLevelPtr->set_val; 
  
  // Display the percentage value on LCD
  lcd.setCursor(12, 0);
  if (waterLevelPtr->percentage < 0) waterLevelPtr->percentage = 0; 
  lcd.print(waterLevelPtr->percentage);
  lcd.print("%  ");

  // Pump control logic based on water level percentage using pointer
  if (waterLevelPtr->percentage < 30 & digitalRead(11)) pump = 1;   
  if (waterLevelPtr->percentage > 99) pump = 0;                     
  digitalWrite(12, !pump);                                          

 
  // Display pump status on LCD
  lcd.setCursor(5, 1);
  if (pump == 1) lcd.print("ON ");
  else if (pump == 0) lcd.print("OFF");

  // Display mode status on LCD (MANUAL or AUTO)
  lcd.setCursor(9, 1);
  if (!digitalRead(11)) lcd.print("MANUAL");
  else lcd.print("AUTO  ");

  // Adjust threshold if button is pressed in AUTO mode
  if (!digitalRead(10) & !state & digitalRead(11)) {
    state = 1;
    waterLevelPtr->set_val = waterLevelPtr->inches;   
    EEPROM.write(0, waterLevelPtr->set_val);          
  }

  // Toggle pump if button is pressed in MANUAL mode
  if (!digitalRead(10) & !state & !digitalRead(11)) {
    state = 1;
    pump = !pump;                       
  }

  // Reset state when button is released
  if (digitalRead(10)) state = 0;

  delay(500);                           
}

// Function to convert microseconds to inches
long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}
