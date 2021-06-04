#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <FastLED.h>
#include <SLIPEncodedSerial.h>

#define X_PIN A0
#define Y_PIN A1
#define BUTTON_PIN 14

#define NUM_LEDS 60
#define DATA_PIN 7

#ifdef ESP32
// The ESP32 has a 12-bit ADC, so we get values from 0 - 4095 instead!
#define ANALOG_MAX_VALUE 4095.0
#else
#define ANALOG_MAX_VALUE 1023.0
#endif

SLIPEncodedSerial SLIPSerial(Serial);
CRGB leds[NUM_LEDS];
int lastButtonState = -1;
float lastX = -1.0;
float lastY = -1.0;



void onXy(OSCMessage& msg) {
  float x = msg.getFloat(0);
  float y = msg.getFloat(1);

  // X will change position, Y will change hue
  int whichLed = round(x * (NUM_LEDS - 1));
  int newHue = round(y * 255);

  leds[whichLed].setHue(newHue);
}

void onToggle(OSCMessage& msg) {
  // Reset everything to red when the button is pressed
  if (msg.getFloat(0) == 1.0) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
    }
  }
}

void receiveOSC() {
  if (SLIPSerial.available() == 0) {
    // Nothing to read right now
    return;
  }

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
    // Print out messages for debugging
    msg.send(Serial);
    Serial.println();

    msg.dispatch("/3/xy", onXy);
    msg.dispatch("/3/toggle1", onToggle);
    // You can handle more addresses here if you like!
  }
}

void sendOSC() {
  float xAxis = ((float) analogRead(X_PIN)) / ANALOG_MAX_VALUE;
  float yAxis = ((float) analogRead(Y_PIN)) / ANALOG_MAX_VALUE;
  int button = digitalRead(BUTTON_PIN);

  // Require a 2% change before we send a new value to avoid sending continuously
  if (abs(xAxis - lastX) >= 0.02 || abs(yAxis - lastY) >= 0.02) {
    // Use the same addresses as page 3 of TouchOSC's Simple layout
    OSCMessage xyMessage("/3/xy");

    // Add both the x and y values to the same message, this makes sure they change together.
    // (Also, this is what TouchOSC does.)
    xyMessage.add(xAxis);
    xyMessage.add(yAxis);

    SLIPSerial.beginPacket();
    xyMessage.send(SLIPSerial);
    SLIPSerial.endPacket();

    lastX = xAxis;
    lastY = yAxis;
  }

  if (lastButtonState != button) {
    // Use the same addresses as page 3 of TouchOSC's Simple layout
    OSCMessage buttonMessage("/3/toggle1");

    // Remember that LOW means pressed for this joystick
    buttonMessage.add(button == HIGH ? 0.0 : 1.0);
    SLIPSerial.beginPacket();
    buttonMessage.send(SLIPSerial);
    SLIPSerial.endPacket();
    lastButtonState = button;
  }
}

void setup() {
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(57600);

  // Set up LEDs
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Start them all out red
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
}

void loop() {
  receiveOSC();
  sendOSC();

  // Call show at the end of the loop to display any changes
  FastLED.show();
  delay(16);
}
