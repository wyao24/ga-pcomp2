// Slightly modified version of the FastLED FirstLight example:
// https://github.com/FastLED/FastLED/blob/master/examples/FirstLight/FirstLight.ino
#include <FastLED.h>

// How many leds are in the strip?
#define NUM_LEDS 72

// Which pin is the data line connected to?
#define DATA_PIN 7

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

void setup() {
  // Set up the LEDs and tell the controller about them
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(40);
}

void loop() {
  // Move a single white led
  for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
    // Turn our current led on to white
    // See https://github.com/FastLED/FastLED/wiki/Pixel-reference for available colors and more
    leds[whiteLed] = CRGB::White;

    // Show the leds (only one of which is set to white, from above)
    FastLED.show();

    // Wait a little bit
    delay(100);

    // Turn our current led back to black for the next loop around
    leds[whiteLed] = CRGB::Black;
  }
}
