#include "wokwi-api.h" // Include Wokwi API for hardware simulation
#include <stdio.h>
#include <stdlib.h>

// Structure to hold the state of the MQ2 chip
typedef struct {
  pin_t pin_ao;          // Analog output pin for gas concentration
  pin_t pin_do;          // Digital output pin for gas detection threshold
  pin_t pin_vcc;         // VCC pin for power supply
  pin_t pin_gnd;         // GND pin for ground connection
  uint32_t gas_attr;     // Attribute for gas concentration percentage
  uint32_t threshold_attr; // Attribute for clean air threshold
} chip_state_t;

// Forward declaration of the timer event handler
static void chip_timer_event(void *user_data);

// Function to initialize the MQ2 chip
void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t)); // Allocate memory for chip state

  // Initialize pins
  chip->pin_ao = pin_init("AO", ANALOG); // Set AO pin as ANALOG input
  chip->gas_attr = attr_init("gas", 10); // Initialize gas attribute with default value
  chip->threshold_attr = attr_init("threshold", 50); // Initialize threshold attribute
  chip->pin_do = pin_init("DO", OUTPUT_LOW); // Set DO pin as OUTPUT (initially LOW)
  chip->pin_vcc = pin_init("VCC", INPUT_PULLDOWN); // Set VCC pin as INPUT_PULLDOWN
  chip->pin_gnd = pin_init("GND", INPUT_PULLUP); // Set GND pin as INPUT_PULLUP

  // Configure timer for periodic updates
  const timer_config_t timer_config = {
    .callback = chip_timer_event, // Set the callback function for the timer
    .user_data = chip, // Pass the chip state to the callback
  };
  timer_t timer_id = timer_init(&timer_config); // Initialize timer
  timer_start(timer_id, 1000, true); // Start timer to call every 1000 ms (1 second)
}

// Timer event handler function
void chip_timer_event(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data; // Cast user_data to chip state

  // Read gas and threshold attributes as float values
  float voltage = (attr_read_float(chip->gas_attr)) * 5.0 / 100; // Convert gas % to voltage
  float threshold_v = (attr_read_float(chip->threshold_attr)) * 5.0 / 100; // Convert threshold % to voltage

  // Check if power is supplied correctly
  if (pin_read(chip->pin_vcc) && !pin_read(chip->pin_gnd)) {
    pin_dac_write(chip->pin_ao, voltage); // Write voltage to AO pin
    // Check if the gas concentration exceeds the threshold
    if (voltage > threshold_v)
      pin_write(chip->pin_do, HIGH); // Set DO pin HIGH if threshold is exceeded
    else
      pin_write(chip->pin_do, LOW); // Set DO pin LOW if below threshold
  }
}
