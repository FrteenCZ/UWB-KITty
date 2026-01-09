#include "StatusLED.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void StatusLED_setup()
{
    strip.begin();
    strip.show(); // LED off
}

void StatusLED_setColor(uint8_t r, uint8_t g, uint8_t b)
{
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}
