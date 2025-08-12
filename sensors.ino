#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// Pins
#define DT2_PIN A0
#define GPS_RX 4
#define GPS_TX 3

// Thresholds
#define MINOR_SPIKE_THRESHOLD 50.0
#define MAJOR_SPIKE_THRESHOLD 70.0

// GPS
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);  // To ESP8266
  gpsSerial.begin(9600);
}

float dt2_raw_to_celsius(int raw) {
  return (raw / 1023.0) * 100.0; // Placeholder — calibrate
}

void loop() {
  // Temp reading
  int raw = analogRead(DT2_PIN);
  float temperature = dt2_raw_to_celsius(raw);

  // GPS reading
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  double lat = gps.location.isValid() ? gps.location.lat() : 0.0;
  double lng = gps.location.isValid() ? gps.location.lng() : 0.0;

  // Event decision
  String eventType = "";
  if (temperature >= MAJOR_SPIKE_THRESHOLD) eventType = "DEPLOY";
  else if (temperature >= MINOR_SPIKE_THRESHOLD) eventType = "WARNING";

  // Send to ESP8266
  if (eventType != "") {
    String out = eventType + "," + String(temperature, 2) + "," + String(lat, 6) + "," + String(lng, 6);
    Serial.println(out);
  }

  delay(2000);
}
