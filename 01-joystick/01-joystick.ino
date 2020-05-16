#define X_PIN A0
#define Y_PIN A1
#define BUTTON_PIN 6

void setup() {
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(57600);
}

void loop() {
  int xAxis = analogRead(X_PIN);
  int yAxis = analogRead(Y_PIN);
  int button = digitalRead(BUTTON_PIN);

  Serial.print("x:");
  Serial.print(xAxis);
  Serial.print(" y:");
  Serial.print(yAxis);
  Serial.print(" button:");
  // The button connects to ground when pressed and has a pull-UP resistor.
  // Therefore, 1 means off and 0 means on.
  Serial.print(button == HIGH ? "off" : "on");
  Serial.println();
}
