#ifndef STATUSLED_H
#define STATUSLED_h

#include <Adafruit_NeoPixel.h>
#include "../config.h"

#define LED_COUNT 1 // Number of LEDs in the strip

void StatusLED_setup();                                   // Initialize the Status LED
void StatusLED_setColor(uint8_t r, uint8_t g, uint8_t b); // Set the color of the Status LED

#endif