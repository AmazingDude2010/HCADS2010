#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>

// Wi-Fi + Blynk
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
const char* ssid = "YOUR_WIFI_SSID";
const char* pass = "YOUR_WIFI_PASSWORD";

// Virtual pins
const int VPIN_TEMP = V0;
const int VPIN_EVENT = V1;
const int VPIN_LAT = V2;
const int VPIN_LNG = V3;
const int VPIN_CONFIRM = V4;

// Data vars
String inputLine = "";
String lastEventType;
float lastTemp;
double lastLat, lastLng;

// Serial link to Drone MCU
SoftwareSerial droneSerial(D5, D6); // RX, TX

void setup() {
  Serial.begin(9600);        // From Nano
  droneSerial.begin(9600);   // To Drone MCU
  Blynk.begin(auth, ssid, pass);
}

bool parseAndPush(String line) {
  line.trim();
  int c1 = line.indexOf(',');
  int c2 = line.indexOf(',', c1 + 1);
  int c3 = line.indexOf(',', c2 + 1);
  if (c1 < 0 || c2 < 0 || c3 < 0) return false;

  lastEventType = line.substring(0, c1);
  lastTemp = line.substring(c1 + 1, c2).toFloat();
  lastLat = line.substring(c2 + 1, c3).toDouble();
  lastLng = line.substring(c3 + 1).toDouble();

  Blynk.virtualWrite(VPIN_TEMP, lastTemp);
  Blynk.virtualWrite(VPIN_EVENT, lastEventType);
  Blynk.virtualWrite(VPIN_LAT, lastLat);
  Blynk.virtualWrite(VPIN_LNG, lastLng);

  if (lastEventType == "WARNING") {
    Blynk.notify("WARNING: " + String(lastTemp, 2) + "°C");
  } 
  else if (lastEventType == "DEPLOY") {
    Blynk.notify("Major spike — Confirm deploy!");
    Blynk.virtualWrite(VPIN_CONFIRM, 1); // Enable button
  }
  return true;
}

BLYNK_WRITE(VPIN_CONFIRM) {
  int value = param.asInt();
  if (value == 1) {
    Blynk.notify("Deploy initiated: " + String(lastLat, 6) + "," + String(lastLng, 6));
    String cmd = "DEPLOY," + String(lastLat, 6) + "," + String(lastLng, 6) + "\n";
    droneSerial.print(cmd); // Send to drone MCU
    Blynk.virtualWrite(VPIN_CONFIRM, 0); // Reset button
  }
}

void loop() {
  Blynk.run();
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseAndPush(inputLine);
      inputLine = "";
    } else if (c != '\r') {
      inputLine += c;
    }
  }
}
