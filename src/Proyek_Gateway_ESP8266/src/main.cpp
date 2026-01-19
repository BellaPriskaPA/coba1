#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LoRa.h>
#include <SPI.h>

// Config Wi-Fi & Firebase dari HTML Anda
#define WIFI_SSID "April"
#define WIFI_PASSWORD "gembulelek"
#define API_KEY "AIzaSyAFUXPpKxImJKhhm5MjQULk9dxFSzrpNIE"
#define DATABASE_URL "https://smarthiker-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); [cite: 303]
  
  // Init LoRa ESP8266 [cite: 293-298]
  LoRa.setPins(15, 16, 5); 
  LoRa.begin(433E6);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String data = "";
    while (LoRa.available()) data += (char)LoRa.read(); [cite: 316]
    
    // Parsing "lat,lon"
    int commaIndex = data.indexOf(',');
    if (commaIndex > 0) {
      String lat = data.substring(0, commaIndex);
      String lon = data.substring(commaIndex + 1);

      // Upload ke Firebase node 'hiker' 
      FirebaseJson json;
      json.set("lat", lat);
      json.set("lon", lon);
      Firebase.RTDB.setJSON(&fbdo, "/hiker", &json);
    }
  }
}