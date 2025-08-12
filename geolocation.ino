String incoming = "";

void setup() {
  Serial.begin(9600); // From ESP8266
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (incoming.startsWith("DEPLOY")) {
        int firstComma = incoming.indexOf(',');
        int secondComma = incoming.indexOf(',', firstComma + 1);
        float lat = incoming.substring(firstComma + 1, secondComma).toFloat();
        float lng = incoming.substring(secondComma + 1).toFloat();

        Serial.print("Deploying drone to: ");
        Serial.print(lat, 6);
        Serial.print(", ");
        Serial.println(lng, 6);

        // Can integrate with drone autopilot here
      }
      incoming = "";
    } else {
      incoming += c;
    }
  }
}
