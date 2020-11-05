#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define X_PIN A9
#define Y_PIN A7
#define BUTTON_PIN 14

#ifdef ESP32
// The ESP32 has a 12-bit ADC, so we get values from 0 - 4095 instead!
#define ANALOG_MAX_VALUE 4095.0
#else
#define ANALOG_MAX_VALUE 1023.0
#endif

char ssid[] = "*****************"; // your network SSID (name)
char pass[] = "*******"; // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(10,10,10,10); // remote IP of your computer
const unsigned int outPort = 9999; // remote port to receive OSC
int lastButtonState = -1;
float lastX = -1.0;
float lastY = -1.0;

void setup() {
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

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
}

void loop() {
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

    Udp.beginPacket(outIp, outPort);
    xyMessage.send(Udp);
    Udp.endPacket();
    lastX = xAxis;
    lastY = yAxis;
  }

  if (lastButtonState != button) {
    OSCMessage buttonMessage("/3/toggle1");

    // Remember that LOW means pressed for this joystick
    buttonMessage.add(button == HIGH ? 0.0 : 1.0);
    Udp.beginPacket(outIp, outPort);
    buttonMessage.send(Udp);
    Udp.endPacket();
    lastButtonState = button;
  }

  delay(16);
}
