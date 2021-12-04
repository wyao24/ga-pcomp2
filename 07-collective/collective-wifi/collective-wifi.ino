#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <FastLED.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#define X_PIN A9
#define Y_PIN A7
#define BUTTON_PIN 14

#define NUM_LEDS 60
#define DATA_PIN 21

#ifdef ESP32
// The ESP32 has a 12-bit ADC, so we get values from 0 - 4095 instead!
#define ANALOG_MAX_VALUE 4095.0
#else
#define ANALOG_MAX_VALUE 1023.0
#endif

// Ensure these match your setup before uploading!
char ssid[] = "*****************"; // your network SSID (name)
char pass[] = "*******"; // your network password
const IPAddress outIp(10,10,10,10); // remote IP of your collective server
const unsigned int outPort = 9000; // remote port of your collective server



// A UDP instance to let us send and receive UDP packets over the network
WiFiUDP network;

// The state of our LED strip
CRGB leds[NUM_LEDS];

// Our unique hue, assigned at startup
int myHue;

// The state of the joystick, used to ensure we only send messages when it changes
int lastButtonState = HIGH;
bool lastCentered = true;



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
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.print("Could not find ");
      Serial.print(ssid);
      Serial.println(", check your SSID");
    }
    else if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST) {
      Serial.print("\nConnection failed! Retrying ");
      Serial.println(ssid);
      WiFi.begin(ssid, pass);
    }
    else {
      Serial.print(".");
    }
    delay(500);
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  // Pick a hue
  myHue = random(256);

  Serial.print("My hue: ");
  Serial.println(myHue);

  // Set up LEDs
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Start with random colors
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random(256), 255, 255);
  }
  FastLED.show();
}

void loop() {
  readMessage();
  sendMessage();

  // Call show at the end of the loop to display any changes
  FastLED.show();
  // Short delay to prevent sending too many OSC messages
  delay(16);
}



void readMessage() {
  OSCMessage oscMessage;

  int size = network.parsePacket();

  if (size > 0) {
    while (size--) {
      oscMessage.fill(network.read());
    }

    if (oscMessage.hasError()) {
      OSCErrorCode error = oscMessage.getError();
      Serial.print("error: ");
      Serial.println(error);
      return;
    }

    // Print out messages for debugging
    oscMessage.send(Serial);
    Serial.println();

    oscMessage.dispatch("/xy", onXy);
    oscMessage.dispatch("/toggle", onToggle);
    // You can handle more addresses here if you like!
  }
}

void sendMessage() {
  float xAxis = ((float) analogRead(X_PIN)) / ANALOG_MAX_VALUE;
  float yAxis = ((float) analogRead(Y_PIN)) / ANALOG_MAX_VALUE;
  int button = digitalRead(BUTTON_PIN);
  bool centered = xAxis > 0.4 && xAxis < 0.6 && yAxis > 0.4 && yAxis < 0.6;

  // Don't send the position continuously when the joystick stays centered
  if (!centered || !lastCentered) {
    OSCMessage xyMessage("/xy");

    // Add both the x and y values to the same message, this makes sure they change together.
    xyMessage.add(xAxis);
    xyMessage.add(yAxis);
    // Add myHue at the end so we can tell players apart
    xyMessage.add(myHue);

    network.beginPacket(outIp, outPort);
    xyMessage.send(network);
    network.endPacket();
    lastCentered = centered;
  }

  if (lastButtonState != button) {
    OSCMessage buttonMessage("/toggle");
    // Remember that LOW means pressed for this joystick
    float buttonValue = button == HIGH ? 0.0 : 1.0;

    buttonMessage.add(buttonValue);
    // Add myHue at the end so we can tell players apart
    buttonMessage.add(myHue);
    network.beginPacket(outIp, outPort);
    buttonMessage.send(network);
    network.endPacket();
    lastButtonState = button;
  }
}

void onXy(OSCMessage& msg) {
  float x = msg.getFloat(0);
  float y = msg.getFloat(1);
  int playerId = msg.getInt(2);
  CRGB playerColor = CHSV(playerId, 255, 255);

  // Uses the joystick to "spread" the player's color.
  // X sets the direction, Y sets the starting position, and the ID tells us the color.

  // If the X axis is in the center, exit the function here and don't change anything.
  // We need a direction.
  if (x > 0.4 && x < 0.6) {
    return;
  }

  // Start at a location determined by the Y axis
  int whichLed = round(y * (NUM_LEDS - 1));

  // If this pixel is already the right color, find the edge of the "block" in the specified
  // direction, stopping if we hit the end of the strip.
  if (x > 0.5) {
    // Find the right edge
    while (whichLed < NUM_LEDS && leds[whichLed] == playerColor) {
      whichLed++;
    }

    // Spread out!
    if (whichLed < NUM_LEDS) {
      leds[whichLed] = playerColor;
    }
  }
  else {
    // Find the left edge
    while (whichLed >= 0 && leds[whichLed] == playerColor) {
      whichLed--;
    }

    // Spread out!
    if (whichLed >= 0) {
      leds[whichLed] = playerColor;
    }
  }
}

void onToggle(OSCMessage& msg) {
  // Turn everything that matches the player's color to black when the button is pressed
  if (msg.getFloat(0) == 1.0) {
    int playerId = msg.getInt(1);
    CRGB playerColor = CHSV(playerId, 255, 255);

    for (int i = 0; i < NUM_LEDS; i++) {
      if (leds[i] == playerColor) {
        leds[i] = CRGB::Black;
      }
    }
  }
}
