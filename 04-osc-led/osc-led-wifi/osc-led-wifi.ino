/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266/ESP32

  Example for receiving open sound control (OSC) messages on the ESP8266/ESP32
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

--------------------------------------------------------------------------------------------- */
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "*****************"; // your network SSID (name)
char pass[] = "*******"; // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 8888; // local port to listen for UDP packets (here's where we send the packets)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(57600);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

}

void led(OSCMessage &msg) {
  // TouchOSC toggles send either 0 or 1
  int ledState = msg.getFloat(0) > 0.0 ? HIGH : LOW;
  digitalWrite(LED_BUILTIN, ledState);
  Serial.print("led:");
  Serial.println(ledState);
}

void loop() {
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      // This will print the raw OSC messages just to make sure you're getting data
      msg.send(Serial);
      Serial.println();
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
    } else {
      OSCErrorCode error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
