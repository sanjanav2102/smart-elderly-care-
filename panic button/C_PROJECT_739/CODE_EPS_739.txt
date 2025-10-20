#include <Arduino.h>

// Define a structure for a GPIO pin
typedef struct {
    uint8_t pinNumber;  // Pin number
    uint8_t mode;       // Pin mode (INPUT/OUTPUT)
} GPIO_Pin;

// Initialize a GPIO pin by setting its mode
void initPin(GPIO_Pin *gpio) {
    pinMode(gpio->pinNumber, gpio->mode);  // Set the mode of the pin
}

// Define GPIO pins for button, LED, and buzzer
GPIO_Pin button = {7, INPUT};   // Button on pin 7
GPIO_Pin led = {10, OUTPUT};     // LED on pin 10
GPIO_Pin buzzer = {13, OUTPUT};  // Buzzer on pin 13

// Function to initialize all GPIO pins
void initAllPins(GPIO_Pin *pins[], int count) {
    for (int i = 0; i < count; i++) {
        initPin(pins[i]);  // Initialize each pin
    }
}

// Function to turn on the LED and play the buzzer tone
void activateLEDAndBuzzer() {
    digitalWrite(led.pinNumber, HIGH);   // Turn on the LED
    tone(buzzer.pinNumber, 1000);        // Play a 1000 Hz tone
}

// Function to turn off the LED and stop the buzzer tone
void deactivateLEDAndBuzzer() {
    digitalWrite(led.pinNumber, LOW);    // Turn off the LED
    noTone(buzzer.pinNumber);            // Stop the tone
}

void setup() {
    // Create an array of pin pointers
    GPIO_Pin *pins[] = {&button, &led, &buzzer};

    // Initialize all GPIO pins
    initAllPins(pins, 3);

    // Start with LED and buzzer off
    deactivateLEDAndBuzzer();
}

void loop() {
    // Check if the button is pressed
    if (digitalRead(button.pinNumber) == HIGH) {
        activateLEDAndBuzzer();  // Activate LED and buzzer
    } else {
        deactivateLEDAndBuzzer(); // Deactivate LED and buzzer
    }

    delay(10);  // Small delay for stability
}
