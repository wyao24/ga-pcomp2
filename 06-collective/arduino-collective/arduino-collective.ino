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

#define NUM_LEDS 72
#define DATA_PIN 21

#ifdef ESP32
// The ESP32 has a 12-bit ADC, so we get values from 0 - 4095 instead!
#define ANALOG_MAX_VALUE 4095.0
#else
#define ANALOG_MAX_VALUE 1023.0
#endif

// Feel free to change this, it will change the number of colored dots on the strip
#define MAX_PLAYERS 8



// ==== SET THESE CONFIGURATION VALUES BEFORE UPLOADING! ====

const char ssid[] = "*****************"; // your network SSID (name)
const char pass[] = "*******"; // your network password

const IPAddress outIp(10,10,10,10); // remote IP of your collective server
const unsigned int outPort = 9000; // remote port of your collective server

const char groupName[] = "change-me"; // pick a unique name for your group
const int myPlayer = 0; // pick a unique number for yourself (0 to groupSize - 1)

// ==== END OF CONFIGURATION ====



// A UDP instance to let us send and receive UDP packets over the network
WiFiUDP network;

// The state of our LED strip
CRGB leds[NUM_LEDS];

// This is our "game state", just remember where each player is currently
int playerPosition[MAX_PLAYERS];

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Ensure the player number isn't out of range
  if (myPlayer >= MAX_PLAYERS) {
    Serial.print("Invalid player number! It must be less than ");
    Serial.print(MAX_PLAYERS);
    Serial.print(", you picked ");
    Serial.println(myPlayer);
    // This just runs forever, stopping execution
    while (true);
  }

  Serial.print("Welcome, ");
  Serial.print(groupName);
  Serial.print(" player ");
  Serial.print(myPlayer);
  Serial.println("!");
  Serial.println();

  // Start all players spaced out evenly along the strip
  for (int i = 0; i < MAX_PLAYERS; i++) {
    playerPosition[i] = NUM_LEDS * getPositionsPerLed() * i / MAX_PLAYERS;
  }

  // Set up LEDs
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(40);

  // Blank the LED strip
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  // Start by dimming what's currently on the LED (the previous "frame")
  fadeToBlackBy(leds, NUM_LEDS, 32);

  showPlayerPositions();
  sendMessage();
  readMessage();

  // Call show at the end of the loop to display any changes
  FastLED.show();
  // Short delay to prevent sending too many OSC messages
  delay(16);
}



void sendMessage() {
  float xAxis = analogRead(X_PIN) / ANALOG_MAX_VALUE;
  float yAxis = analogRead(Y_PIN) / ANALOG_MAX_VALUE;
  int button = digitalRead(BUTTON_PIN);
  bool centered = xAxis > 0.45 && xAxis < 0.55 && yAxis > 0.45 && yAxis < 0.55;

  // Don't send the position continuously when the joystick stays centered
  if (!centered || !lastCentered) {
    OSCMessage xyMessage("/xy");

    // Add group and player at the beginning so we can tell where messages are coming from
    xyMessage.add(groupName);
    xyMessage.add(myPlayer);

    // Add both the x and y values to the same message, this makes sure they change together.
    // If we're centered, send the actual center (it looks nicer that way)
    if (centered) {
      xyMessage.add(0.5f);
      xyMessage.add(0.5f);
    }
    else {
      xyMessage.add(xAxis);
      xyMessage.add(yAxis);
    }

    network.beginPacket(outIp, outPort);
    xyMessage.send(network);
    network.endPacket();

    Serial.print("OUT ");
    xyMessage.send(Serial);
    Serial.println();

    lastCentered = centered;
  }

  if (lastButtonState != button) {
    OSCMessage buttonMessage("/toggle");
    // Remember that LOW means pressed for this joystick
    float buttonValue = button == HIGH ? 0.0 : 1.0;

    // Add group and player at the beginning so we can tell where messages are coming from
    buttonMessage.add(groupName);
    buttonMessage.add(myPlayer);
    buttonMessage.add(buttonValue);

    network.beginPacket(outIp, outPort);
    buttonMessage.send(network);
    network.endPacket();

    Serial.print("OUT ");
    buttonMessage.send(Serial);
    Serial.println();

    lastButtonState = button;
  }
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
    Serial.print("IN  ");
    oscMessage.send(Serial);
    Serial.println();

    // Handle the expected messages
    oscMessage.dispatch("/xy", onXy);
    oscMessage.dispatch("/toggle", onToggle);
  }
}



// ==== These functions map the controls to game behavior ====

void onXy(OSCMessage& msg) {
  // This message has the following values:
  // 0: group name (string, should always be the same as groupName, so you shouldn't need it)
  // 1: player number (int, 0 to MAX_PLAYERS - 1)
  // 2: x position (float, 0 to 1)
  // 3: y position (float, 0 to 1)
  // Double-check the player number is less than MAX_PLAYERS to prevent crashes
  int player = msg.getInt(1) % MAX_PLAYERS;
  float xAxis = msg.getFloat(2);
  float yAxis = msg.getFloat(3);
  CRGB playerColor = getPlayerColor(player);
  int curPosition = playerPosition[player];

  // Uses the joystick to move player's dot around.
  // X moves the dot, Y does nothing, and the player number tells us what color to use.

  if (xAxis <= 0.45) {
    // Move left, stopping at the beginning
    playerPosition[player] = max(0, curPosition - 1);
  }
  else if (xAxis >= 0.55) {
    // Move right, stopping at the end
    playerPosition[player] = min(curPosition + 1, NUM_LEDS * getPositionsPerLed() - 1);
  }
  // No else case -- this would mean the X axis is in the center, and we don't have anything to do
  // if we don't have a direction to go.
}

void onToggle(OSCMessage& msg) {
  // This message has the following values:
  // 0: group name (string, should always be the same as groupName, so you shouldn't need it)
  // 1: player number (int, 0 to MAX_PLAYERS - 1)
  // 2: button state (float, 0 for released, 1 for pressed)
  int player = msg.getInt(1) % MAX_PLAYERS;
  bool isPressed = msg.getFloat(2) == 1.0;

  // If someone presses the button while close to other players, paint the area with their combined
  // colors.
  if (isPressed) {
    bool collision = false;
    CRGB paintColor = getPlayerColor(player);

    for (int otherPlayer = 0; otherPlayer < MAX_PLAYERS; otherPlayer++) {
      if (otherPlayer != player && isCollision(player, otherPlayer)) {
        collision = true;
        paintColor = blendColors(paintColor, getPlayerColor(otherPlayer));
      }
    }

    if (collision) {
      showCollision(playerPosition[player], paintColor);
    }
  }
}



// ==== These functions determine the game look and feel ====

// This function converts a player number to a color. Feel free to change it however you want!
// The implementation below evenly spaces the player colors across the hue spectrum based on
// MAX_PLAYERS. The hue chart can be found here:
// https://raw.githubusercontent.com/FastLED/FastLED/gh-pages/images/HSV-rainbow-with-desc.jpg
CRGB getPlayerColor(int player) {
  return CHSV(player * (256 / MAX_PLAYERS), 255, 255);
}

// This function determines the number of player "positions" per LED. As written, the player moves
// by at most one "position" on each update, and there at most 60 updates per second. So changing
// the positions per LED will change the players' movement speed. The larger this number, the slower
// everyone will move.
int getPositionsPerLed() {
  return 15;
}

// This function determines how "collisions" work, that is, how we decide whether two players are
// close enough to each other for a button push to do something. This implementation will return
// true if they are on the same LED.
bool isCollision(int p1, int p2) {
  return playerPosition[p1] / getPositionsPerLed() == playerPosition[p2] / getPositionsPerLed();
}

// Determines how to combine multiple player colors. Note that it may be applied multiple times
// if more than two players collide!
CRGB blendColors(CRGB curColor, CRGB newColor) {
  // blend() comes from FastLED. The third number is how much to blend, from 0-255.
  // See https://fastled.io/docs/3.1/group___colorutils.html for more color utility functions.
  return blend(curColor, newColor, 127);
}

// Display the player positions on the LED strip. This implementation draws a dot at the player
// position.
void showPlayerPositions() {
  for (int curPlayer = 0; curPlayer < MAX_PLAYERS; curPlayer++) {
    int curPosition = playerPosition[curPlayer];
    CRGB playerColor = getPlayerColor(curPlayer);
    int ledAtPosition = curPosition / getPositionsPerLed();

    if (leds[ledAtPosition] == CRGB(0, 0, 0)) { // Black
      leds[ledAtPosition] = playerColor;
    }
    else {
      leds[ledAtPosition] = blendColors(leds[ledAtPosition], playerColor);
    }
  }
}

// Update the LED strip to display a collision. This implementation paints the neighboring few LEDs.
// Splat!
void showCollision(int position, CRGB paintColor) {
  int ledAtPosition = position / getPositionsPerLed();

  for (int i = max(0, ledAtPosition - 6); i <= ledAtPosition + 6 && i < NUM_LEDS; i++) {
    leds[i] = paintColor;
  }
}
