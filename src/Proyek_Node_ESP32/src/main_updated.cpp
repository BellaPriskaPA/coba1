// ========================================
// 🚀 UPDATED ESP32 NODE - SEND HIKER NAME
// ========================================
// File: src/Proyek_Node_ESP32/src/main.cpp
// Update: Kirim data dengan format "hikerName,lat,lon"
// ========================================

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BMP280.h>
#include <LoRa.h>
#include <SPI.h>
#include "DHT.h"

// ========== IDENTIFIER PENDAKI ==========
// PENTING: Ganti ini sesuai nama pendaki yang akan track
#define HIKER_NAME "bella"  // ← Ubah ke: "bella", "budi", "rina", dll
#define DEVICE_ID 1

// ========== GPIO PINS ==========
#define LORA_SS 5       // LoRa chip select
#define LORA_RST 14     // LoRa reset
#define LORA_DIO0 2     // LoRa interrupt
#define DHTPIN 4        // DHT sensor pin
#define DHTTYPE DHT22

// ========== SENSORS ==========
Adafruit_MPU6050 mpu;
Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

// ========== GPS (Simulated or Real) ==========
// Jika punya GPS module, uncomment dan config sesuai
// #include <TinyGPS++.h>
// #include <HardwareSerial.h>
// TinyGPS++ gps;
// HardwareSerial ss(1);  // Serial 1 untuk GPS

// Untuk testing: simulasi GPS dengan increment kecil
float currentLat = -7.50;
float currentLon = 110.50;

// ========== TIMING ==========
unsigned long lastSensorReadTime = 0;
unsigned long lastLoRaSendTime = 0;
const unsigned long SENSOR_READ_INTERVAL = 1000;   // 1 second
const unsigned long LORA_SEND_INTERVAL = 3000;     // 3 seconds

// ========== DATA STRUCT ==========
struct SensorData {
  float latitude;
  float longitude;
  float altitude;
  float temperature;
  float humidity;
  float accelX, accelY, accelZ;
  uint32_t timestamp;
};

SensorData sensorData;

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Wait for Serial
  delay(500);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  SmartHiker Node (ESP32) v2.0          ║");
  Serial.println("║  Multi-Sensor LoRa Transmitter         ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.print("👤 Hiker Name: ");
  Serial.println(HIKER_NAME);
  Serial.print("🆔 Device ID: ");
  Serial.println(DEVICE_ID);
  
  // ========== INIT I2C SENSORS ==========
  Serial.println("\n📊 Initializing sensors...");
  
  // MPU6050
  if (!mpu.begin()) {
    Serial.println("✗ MPU6050 not found!");
  } else {
    Serial.println("✓ MPU6050 ready");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
  
  // BMP280
  unsigned int BMPStatus;
  BMPStatus = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  if (!BMPStatus) {
    Serial.println("✗ BMP280 not found!");
  } else {
    Serial.println("✓ BMP280 ready");
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  }
  
  // DHT22
  dht.begin();
  Serial.println("✓ DHT22 ready");
  
  // ========== INIT LORA ==========
  Serial.println("\n📡 Initializing LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(433E6)) {  // 433 MHz
    Serial.println("✗ LoRa init FAILED!");
    while (1);  // Halt
  }
  
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(7);
  LoRa.setCodingRate4(8);
  
  Serial.println("✓ LoRa ready (433 MHz, SF7)");
  
  // ========== GPS SETUP (If Real GPS) ==========
  // Uncomment jika punya GPS module
  // ss.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  // Serial.println("✓ GPS Serial ready");
  
  Serial.println("\n═════════════════════════════════════════");
  Serial.println("Starting sensor loop...\n");
}

// ========== MAIN LOOP ==========
void loop() {
  // ========== READ SENSORS ==========
  if (millis() - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    readAllSensors();
    lastSensorReadTime = millis();
  }
  
  // ========== SEND VIA LORA ==========
  if (millis() - lastLoRaSendTime >= LORA_SEND_INTERVAL) {
    sendLoRaPacket();
    lastLoRaSendTime = millis();
  }
  
  // ========== GPS PROCESSING (If Real GPS) ==========
  // while (ss.available() > 0) {
  //   if (gps.encode(ss.read())) {
  //     if (gps.location.isValid()) {
  //       currentLat = gps.location.lat();
  //       currentLon = gps.location.lng();
  //     }
  //   }
  // }
  
  delay(10);
}

// ========== READ ALL SENSORS ==========
void readAllSensors() {
  // Read MPU6050
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  sensorData.accelX = a.acceleration.x;
  sensorData.accelY = a.acceleration.y;
  sensorData.accelZ = a.acceleration.z;
  
  // Read BMP280
  sensorData.temperature = bmp.readTemperature();
  sensorData.altitude = bmp.readAltitude(1013.25);  // Sea level pressure
  
  // Read DHT22
  sensorData.humidity = dht.readHumidity();
  if (isnan(sensorData.humidity)) {
    sensorData.humidity = 0;
  }
  
  // ========== GPS READING ==========
  // Option 1: Real GPS (uncomment if punya)
  // sensorData.latitude = gps.location.isValid() ? gps.location.lat() : currentLat;
  // sensorData.longitude = gps.location.isValid() ? gps.location.lng() : currentLon;
  
  // Option 2: Simulated (for testing)
  // Increment sedikit untuk simulasi pergerakan
  currentLat += (random(-5, 5) / 10000.0);
  currentLon += (random(-5, 5) / 10000.0);
  
  sensorData.latitude = currentLat;
  sensorData.longitude = currentLon;
  
  sensorData.timestamp = millis();
  
  // ========== DEBUG OUTPUT ==========
  Serial.print("📍 Lat: ");
  Serial.print(sensorData.latitude, 6);
  Serial.print(" | Lon: ");
  Serial.print(sensorData.longitude, 6);
  Serial.print(" | Alt: ");
  Serial.print(sensorData.altitude, 1);
  Serial.print("m | Temp: ");
  Serial.print(sensorData.temperature, 1);
  Serial.println("°C");
}

// ========== SEND LORA PACKET ==========
void sendLoRaPacket() {
  // Format: "hikerName,latitude,longitude"
  // Example: "bella,-7.50,110.50"
  
  String payload = "";
  payload += HIKER_NAME;
  payload += ",";
  payload += String(sensorData.latitude, 6);
  payload += ",";
  payload += String(sensorData.longitude, 6);
  
  // Optional: Add sensor data if you want to log more
  // payload += ",";
  // payload += String(sensorData.altitude, 1);
  // payload += ",";
  // payload += String(sensorData.temperature, 1);
  
  Serial.print("📤 Sending: ");
  Serial.println(payload);
  
  // ========== TRANSMIT ==========
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();
  
  Serial.println("✓ Packet sent\n");
}

// ========================================
// 📋 SETUP INSTRUCTIONS:
// ========================================
//
// 1. HIKER_NAME Configuration:
//    - Line 15: #define HIKER_NAME "bella"
//    - Change "bella" to device's hiker name
//    - Must match ESP32 node (no spaces)
//    - Example names: "bella", "budi", "rina", "ricky"
//
// 2. Data Format Transmitted:
//    Format: "hikerName,latitude,longitude"
//    Example: "bella,-7.50,110.50"
//    This is received by Gateway and uploaded to:
//    Firebase: /hikers/bella { lat: -7.50, lon: 110.50 }
//
// 3. GPS Setup (If Using Real GPS):
//    a. Uncomment lines for GPS initialization
//    b. Connect GPS to UART1:
//       - RX: GPIO 16 (or adjust)
//       - TX: GPIO 17 (or adjust)
//    c. Update RX_PIN and TX_PIN values
//    d. Uncomment GPS reading loop
//
// 4. Sensor Pins (Customize if Different):
//    - MPU6050: I2C (21=SDA, 22=SCL)
//    - BMP280: I2C (21=SDA, 22=SCL)
//    - DHT22: GPIO 4 (adjust DHTPIN if needed)
//    - LoRa: See LORA_ defines
//
// 5. Testing:
//    - Open Serial Monitor (115200)
//    - Check if "Sending:" messages appear
//    - Verify format is correct
//    - Use different HIKER_NAME for each device
//
// 6. Deployment:
//    - Compile & Flash to ESP32
//    - Repeat for other hikers with different HIKER_NAME
//    - Place devices in backpack/harness
//    - Start tracking from Web Dashboard!
//
// ========================================
