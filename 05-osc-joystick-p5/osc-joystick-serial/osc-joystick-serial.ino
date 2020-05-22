#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

#define X_PIN A0
#define Y_PIN A1
#define BUTTON_PIN 6

#ifdef ESP32
// The ESP32 has a 12-bit ADC, so we get values from 0 - 4095 instead!
#define ANALOG_MAX_VALUE 4095.0
#else
#define ANALOG_MAX_VALUE 1023.0
#endif

SLIPEncodedSerial SLIPSerial(Serial);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  SLIPSerial.begin(57600);
}

void loop() {
  float xAxis = ((float) analogRead(X_PIN)) / ANALOG_MAX_VALUE;
  float yAxis = ((float) analogRead(Y_PIN)) / ANALOG_MAX_VALUE;
  int button = digitalRead(BUTTON_PIN);

  // Use the same addresses as page 3 of TouchOSC's Simple layout
  OSCMessage xyMessage("/3/xy");
  OSCMessage buttonMessage("/3/toggle1");

  // Add both the x and y values to the same message, this makes sure they change together.
  // (Also, this is what TouchOSC does.)
  xyMessage.add(xAxis);
  xyMessage.add(yAxis);

  SLIPSerial.beginPacket();
  xyMessage.send(SLIPSerial);
  SLIPSerial.endPacket();

  // Remember that LOW means pressed for this joystick
  buttonMessage.add(button == HIGH ? 0.0 : 1.0);
  SLIPSerial.beginPacket();
  buttonMessage.send(SLIPSerial);
  SLIPSerial.endPacket();

  delay(100);
}
