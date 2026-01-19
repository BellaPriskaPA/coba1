// ========================================
// 🚀 UPDATED ESP8266 GATEWAY - MULTI-USER SUPPORT
// ========================================
// File: src/Proyek_Gateway_ESP8266/src/main.cpp
// Update: Sesuaikan dengan path /hikers/{hikerName}
// ========================================

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LoRa.h>
#include <SPI.h>

// ========== CONFIG WI-FI & FIREBASE ==========
#define WIFI_SSID "NAMA_WIFI_ANDA"
#define WIFI_PASSWORD "PASSWORD_WIFI_ANDA"
#define API_KEY "AIzaSyAFUXPpKxImJKhhm5MjQULk9dxFSzrpNIE"
#define DATABASE_URL "https://smarthiker-default-rtdb.asia-southeast1.firebasedatabase.app"

// ========== GPIO CONFIG ==========
#define LORA_SS 15      // LoRa chip select
#define LORA_RST 16     // LoRa reset
#define LORA_DIO0 5     // LoRa interrupt

// ========== GLOBAL VARIABLES ==========
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 2000;  // Update every 2 seconds

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Tunggu Serial siap
  delay(500);
  Serial.println("\n\n╔══════════════════════════════════════╗");
  Serial.println("║  SmartHiker Gateway (ESP8266) v2.0   ║");
  Serial.println("║  Multi-User LoRa to Firebase         ║");
  Serial.println("╚══════════════════════════════════════╝\n");
  
  // ========== INIT WI-FI ==========
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("📶 Connecting to WiFi: ");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✓ Connected!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" ✗ Failed! Will retry...");
  }
  
  // ========== INIT FIREBASE ==========
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;  // Untuk monitoring token
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println("🔥 Firebase initialized\n");
  
  // ========== INIT LORA ==========
  Serial.println("📡 Initializing LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(433E6)) {  // 433 MHz untuk Southeast Asia
    Serial.println("✗ LoRa init FAILED! Check wiring.");
    while (1);  // Halt
  }
  
  LoRa.setTxPower(20);       // Max power
  LoRa.setSpreadingFactor(7); // SF7 untuk range sedang
  LoRa.setCodingRate4(8);     // Coding rate 4/8
  
  Serial.println("✓ LoRa ready (433 MHz, SF7)\n");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Waiting for LoRa packets...\n");
}

// ========== MAIN LOOP ==========
void loop() {
  // Check Firebase connection
  if (millis() - lastUpdateTime > 30000) {  // Reconnect attempt every 30s
    if (!Firebase.ready()) {
      Serial.println("⚠️  Firebase not ready, reconnecting...");
      WiFi.reconnect();
    }
    lastUpdateTime = millis();
  }
  
  // ========== CHECK LORA PACKET ==========
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String rawData = "";
    
    // Read all incoming bytes
    while (LoRa.available()) {
      rawData += (char)LoRa.read();
    }
    
    Serial.print("📨 Packet received: ");
    Serial.println(rawData);
    Serial.print("   Signal: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
    
    // ========== PARSE DATA ==========
    // Format: "hikerName,latitude,longitude"
    // Example: "bella,-7.5,110.5"
    
    if (parseAndUpload(rawData)) {
      Serial.println("✓ Data uploaded to Firebase\n");
    } else {
      Serial.println("✗ Parse failed! Invalid format\n");
    }
  }
}

// ========== PARSE & UPLOAD FUNCTION ==========
bool parseAndUpload(String data) {
  // Find delimiters
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  
  // Validate format
  if (firstComma < 0 || secondComma < 0 || firstComma == 0) {
    Serial.println("   ✗ Format error: Expected 'name,lat,lon'");
    return false;
  }
  
  // Extract components
  String hikerName = data.substring(0, firstComma);
  String latStr = data.substring(firstComma + 1, secondComma);
  String lonStr = data.substring(secondComma + 1);
  
  // Trim whitespace
  hikerName.trim();
  latStr.trim();
  lonStr.trim();
  
  // Validate values
  float lat = latStr.toFloat();
  float lon = lonStr.toFloat();
  
  if (lat == 0.0 || lon == 0.0) {
    Serial.print("   ✗ Invalid coordinates: ");
    Serial.print(lat);
    Serial.print(", ");
    Serial.println(lon);
    return false;
  }
  
  if (hikerName.length() == 0 || hikerName.length() > 20) {
    Serial.println("   ✗ Invalid hiker name");
    return false;
  }
  
  // ========== BUILD FIREBASE PATH ==========
  // Path: /hikers/{hikerName}
  String path = "/hikers/" + hikerName;
  
  // ========== CREATE JSON ==========
  FirebaseJson json;
  json.set("lat", String(lat, 6));          // 6 decimal places
  json.set("lon", String(lon, 6));
  json.set("timestamp", millis());
  json.set("rssi", LoRa.packetRssi());      // Signal strength
  
  // ========== UPLOAD TO FIREBASE ==========
  Serial.print("   Uploading to: ");
  Serial.println(path);
  
  if (!Firebase.ready()) {
    Serial.println("   ⚠️  Firebase not ready!");
    return false;
  }
  
  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    Serial.print("   📊 ");
    Serial.print(hikerName);
    Serial.print(" [");
    Serial.print(lat, 4);
    Serial.print(", ");
    Serial.print(lon, 4);
    Serial.println("]");
    return true;
  } else {
    Serial.print("   ✗ Firebase error: ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

// ========== FIREBASE CALLBACK (Optional) ==========
void tokenStatusCallback(struct esp_jwt_response_t info) {
  if (info.status == esp_jwt_response_status::ESP_JWT_OK) {
    Serial.println("🔐 Firebase token refreshed");
  } else {
    Serial.println("⚠️  Firebase token error");
  }
}

// ========================================
// 📋 NOTES FOR IMPLEMENTATION:
// ========================================
// 
// 1. Data Format:
//    - Must be: "hikerName,latitude,longitude"
//    - Example: "bella,-7.50,110.50"
//    - No spaces before/after commas (handled by trim())
//
// 2. Firebase Structure:
//    {
//      "hikers": {
//        "bella": { "lat": -7.50, "lon": 110.50, ... },
//        "budi": { "lat": -7.51, "lon": 110.51, ... }
//      }
//    }
//
// 3. Web Dashboard:
//    - Listens to /hikers (all users)
//    - Automatically detects new hikers
//    - Dropdown updates in real-time
//
// 4. Troubleshooting:
//    - Open Serial Monitor (115200 baud)
//    - Check for "Format error" messages
//    - Verify LoRa signal (RSSI value)
//    - Test with hardcoded data first
//
// ========================================
