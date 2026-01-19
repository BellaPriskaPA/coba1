#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>

// Pin LoRa ESP32
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 14
#define DIO0 4
#define SOS_PIN 2

TinyGPSPlus gps;
HardwareSerial SerialGPS(2); 
double lastLat = 0, lastLon = 0;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); [cite: 209]
  pinMode(SOS_PIN, INPUT_PULLUP);
  
  LoRa.setPins(SS, RST, DIO0); [cite: 199]
  if (!LoRa.begin(433E6)) {
    while (1);
  }
}

// Algoritma Haversine [cite: 157-160]
double getDistance(double lat1, double lon1, double lat2, double lon2) {
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat/2) * sin(dLat/2) + cos(radians(lat1)) * cos(radians(lat2)) * sin(dLon/2) * sin(dLon/2);
  return 2 * atan2(sqrt(a), sqrt(1-a)) * 6371000;
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  if (gps.location.isValid()) {
    double currentLat = gps.location.lat();
    double currentLon = gps.location.lng();
    
    // Edge Processing: Filter Jarak 10 Meter [cite: 85, 307]
    if (getDistance(lastLat, lastLon, currentLat, currentLon) > 10.0 || digitalRead(SOS_PIN) == LOW) {
      String payload = String(currentLat, 6) + "," + String(currentLon, 6);
      if (digitalRead(SOS_PIN) == LOW) payload += ",SOS"; [cite: 87, 313]
      
      LoRa.beginPacket();
      LoRa.print(payload);
      LoRa.endPacket();
      
      lastLat = currentLat;
      lastLon = currentLon;
    }
  }
}