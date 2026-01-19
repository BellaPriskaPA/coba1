// ========================================
// 🌐 UPDATED ESP8266 GATEWAY v2.0
// ========================================
// File: src/Proyek_Gateway_ESP8266/src/main_updated.cpp
// Feature: Send LoRa data to Web Backend Server
// ========================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// ========== WIFI CONFIG ==========
#define WIFI_SSID "April"         // ← GANTI SSID WiFi Anda
#define WIFI_PASSWORD "gembulelek" // ← GANTI Password WiFi

// ========== BACKEND SERVER CONFIG ==========
#define BACKEND_HOST "192.168.1.103"   // ← GANTI IP Laptop (cek dengan ipconfig)
#define BACKEND_PORT 3000
#define BACKEND_API "/api/hiker-data"

// ========== GPIO PINS (ESP8266 LoRa Shield) ==========
#define LORA_SS 15      // D8 - Chip Select
#define LORA_RST 0      // D3 - Reset
#define LORA_DIO0 4     // D2 - Interrupt

// ========== LORA PACKET STRUCTURE ==========
struct LoRaPacket {
  char hikerName[32];
  float latitude;
  float longitude;
  float altitude;
  float temperature;
  float humidity;
  uint32_t timestamp;
};

// ========== GLOBAL VARIABLES ==========
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 5000; // 5 detik
WiFiClient wifiClient;
HTTPClient http;

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  SmartHiker Gateway (ESP8266) v2.0     ║");
  Serial.println("║  LoRa Receiver + WiFi Uplink           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // ========== INIT LORA ==========
  Serial.println("📡 Initializing LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(433E6)) {  // 433 MHz
    Serial.println("✗ LoRa init FAILED!");
    while (1);
  }
  
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(7);
  LoRa.setCodingRate4(8);
  Serial.println("✓ LoRa ready (433 MHz, SF7)");
  
  // ========== INIT WIFI ==========
  Serial.println("\n📶 Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected!");
    Serial.print("📍 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi connection FAILED");
    // Continue anyway, akan retry di loop
  }
  
  Serial.println("\n📍 Backend Server: http://" + String(BACKEND_HOST) + ":" + String(BACKEND_PORT));
  Serial.println("═════════════════════════════════════════");
  Serial.println("Listening for LoRa packets...\n");
}

// ========== MAIN LOOP ==========
void loop() {
  // Maintain WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi disconnected, reconnecting...");
    WiFi.reconnect();
  }
  
  // ========== RECEIVE LORA PACKET ==========
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    receiveLoRaPacket();
  }
  
  delay(10);
}

// ========== RECEIVE LORA PACKET ==========
void receiveLoRaPacket() {
  Serial.println("\n📡 LoRa packet received!");
  
  LoRaPacket packet;
  memset(&packet, 0, sizeof(packet));
  
  // Read hiker name
  int i = 0;
  while (LoRa.available() && i < 31) {
    char c = (char)LoRa.read();
    if (c == ',' || c == '\n') break;
    packet.hikerName[i++] = c;
  }
  packet.hikerName[i] = '\0';
  
  // Read latitude
  String latStr = "";
  while (LoRa.available()) {
    char c = (char)LoRa.read();
    if (c == ',') break;
    latStr += c;
  }
  packet.latitude = latStr.toFloat();
  
  // Read longitude
  String lonStr = "";
  while (LoRa.available()) {
    char c = (char)LoRa.read();
    if (c == ',' || c == '\n') break;
    lonStr += c;
  }
  packet.longitude = lonStr.toFloat();
  
  // Read remaining data (altitude, temp, humidity)
  packet.altitude = 500.0;  // Default atau parse dari packet
  packet.temperature = 25.0; // Default atau parse dari packet
  packet.humidity = 60.0;    // Default atau parse dari packet
  packet.timestamp = millis();
  
  // Display received data
  Serial.print("👤 Hiker: ");
  Serial.println(packet.hikerName);
  Serial.print("📍 Position: ");
  Serial.print(packet.latitude, 6);
  Serial.print(", ");
  Serial.println(packet.longitude, 6);
  Serial.print("📏 RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.println(" dBm");
  
  // ========== SEND TO BACKEND ==========
  sendToBackend(packet);
}

// ========== SEND TO BACKEND SERVER ==========
void sendToBackend(LoRaPacket packet) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected, cannot send");
    return;
  }
  
  // Build URL
  String url = "http://" + String(BACKEND_HOST) + ":" + String(BACKEND_PORT) + String(BACKEND_API);
  
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["hikerName"] = packet.hikerName;
  doc["latitude"] = packet.latitude;
  doc["longitude"] = packet.longitude;
  doc["altitude"] = packet.altitude;
  doc["temperature"] = packet.temperature;
  doc["humidity"] = packet.humidity;
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  Serial.print("📤 Sending to backend: ");
  Serial.println(jsonPayload);
  
  // Send HTTP POST
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    Serial.print("✅ HTTP Response: ");
    Serial.println(httpResponseCode);
    
    String response = http.getString();
    Serial.print("📥 Response: ");
    Serial.println(response);
  } else {
    Serial.print("❌ HTTP Error: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}
