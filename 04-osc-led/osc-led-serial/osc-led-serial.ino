#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

SLIPEncodedSerial SLIPSerial(Serial);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  SLIPSerial.begin(57600);
}

void led(OSCMessage &msg) {
  // TouchOSC toggles send either 0 or 1
  int ledState = msg.getFloat(0) > 0.0 ? HIGH : LOW;
  digitalWrite(LED_BUILTIN, ledState);
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
