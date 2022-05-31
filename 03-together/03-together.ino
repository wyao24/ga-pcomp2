#include <FastLED.h>

// Joystick pins
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1
#define JOYSTICK_BUTTON_PIN 6

// How many leds are in the strip?
#define NUM_LEDS 72

// Which pin is the LED strip data line connected to?
#define LED_DATA_PIN 7

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// Used by the sample code
int curLed = 0;

void setup() {
  // Set up the joystick pins
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT);

  Serial.begin(57600);

  // Set up the LEDs and tell the controller about them
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(40);
}

void loop() {
  int xAxis = analogRead(JOYSTICK_X_PIN);
  int yAxis = analogRead(JOYSTICK_Y_PIN);
  int button = digitalRead(JOYSTICK_BUTTON_PIN);

  // Make the joystick control the LED strip in any way you like!
  // Useful documentation: https://github.com/FastLED/FastLED/wiki/Pixel-reference
  // A couple examples (comment in just one):
  moveOne(xAxis, yAxis, button);
  //rotateRainbow(xAxis, yAxis, button);
}

void moveOne(int xAxis, int yAxis, int button) {
  // 700 is an arbitrary value between 512 and 1023 to detect when the joystick moved up
  if (xAxis > 700) {
    leds[curLed] = CRGB::Black;
    curLed = curLed >= (NUM_LEDS - 1) ? 0 : (curLed + 1);
    leds[curLed] = CRGB::Red;
  }
  // 300 is an arbitrary value between 0 and 512 to detect when the joystick moved down
  else if (xAxis < 300) {
    leds[curLed] = CRGB::Black;
    curLed = curLed <= 0 ? (NUM_LEDS - 1) : (curLed - 1);
    leds[curLed] = CRGB::Green;
  }
  FastLED.show();
  delay(50);
}

void rotateRainbow(int xAxis, int yAxis, int button) {
  if (xAxis > 700) {
    curLed = curLed >= (NUM_LEDS - 1) ? 0 : (curLed + 1);
  }
  else if (xAxis < 300) {
    curLed = curLed <= 0 ? (NUM_LEDS - 1) : (curLed - 1);
  }

  int hue = 0;

  for (int i = 0; i < NUM_LEDS; i++) {
    int nextLed = (curLed + i) % NUM_LEDS;

    leds[nextLed] = CHSV(hue, 255, 255);
    hue += 255 / NUM_LEDS;
  }

  FastLED.show();
  delay(50);
}
