#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

#include <FastLED.h>

// How many leds are in the strip?
#define NUM_LEDS 60

// Which pin is the data line connected to?
#define DATA_PIN 7

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

SLIPEncodedSerial SLIPSerial(Serial);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  SLIPSerial.begin(57600);

  // Set up the LEDs and tell the controller about them
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
}

void led(OSCMessage &msg) {
  bool turnOn = msg.getFloat(0) > 0.0;
  Serial.print("led:");
  Serial.println(turnOn);

  // Update the whole strip
  for (int i = 0; i < NUM_LEDS; i++) {
    // TouchOSC toggles send either 0.0 or 1.0
    // See https://github.com/FastLED/FastLED/wiki/Pixel-reference for available colors and more
    leds[i] = turnOn ? CRGB::Red : CRGB::Black;
  }
  FastLED.show();
}

void loop() {
  OSCMessage msg;

  while (!SLIPSerial.endofPacket()) {
    int size = SLIPSerial.available();

    if (size > 0) {
       while(size--) {
          msg.fill(SLIPSerial.read());
       }
     }
  }

  if (!msg.hasError()) {
    // TouchOSC "Simple" Layout, Page 1
    // All of these are "float" values between 0 and 1
    // Faders:
    // "/1/fader1"
    // "/1/fader2"
    // "/1/fader3"
    // "/1/fader4"
    // "/1/fader5"
    // Toggles:
    // "/1/toggle1"
    // "/1/toggle2"
    // "/1/toggle3"
    // "/1/toggle4"
    msg.dispatch("/1/toggle1", led);
  }
}
