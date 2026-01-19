# 🚀 Quick Start - Jalankan Web Dashboard

## 📋 Prerequisites
- Node.js 14+ (download dari nodejs.org)
- Browser modern (Chrome, Firefox, Safari, Edge)
- WiFi connection

---

## 🎯 3 Cara Menjalankan

### **Cara 1: Instant (No Backend - Untuk Testing)**
Hanya buka file HTML langsung:

```bash
# Windows:
start C:\IOT\coba1\src\Web_Dashboard\index.html

# MacOS:
open ~/IOT/coba1/src/Web_Dashboard/index.html

# Linux:
xdg-open ~/IOT/coba1/src/Web_Dashboard/index.html
```

✅ **Kelebihan:** Instant, tidak perlu install  
❌ **Kekurangan:** Data hardcoded, tidak real-time

---

### **Cara 2: Local HTTP Server (Dev Mode)**

#### **Windows (Python):**
```powershell
cd C:\IOT\coba1\src\Web_Dashboard
python -m http.server 8000

# Buka browser: http://localhost:8000
```

#### **Windows (Node.js):**
```powershell
cd C:\IOT\coba1\src\Web_Dashboard
npx http-server -p 8000

# Buka browser: http://localhost:8000
```

#### **MacOS/Linux:**
```bash
cd ~/IOT/coba1/src/Web_Dashboard
python3 -m http.server 8000

# Buka browser: http://localhost:8000
```

✅ **Kelebihan:** Simple, tidak perlu backend  
❌ **Kekurangan:** Data hanya hardcoded

---

### **Cara 3: Full Backend Server (Recommended - Real-time Data)**

#### **Step 1: Install Dependencies**
```bash
cd C:\IOT\coba1
npm install
# Tunggu sampai selesai (~1-2 menit)
```

#### **Step 2: Run Backend Server**
```bash
# Windows:
cd C:\IOT\coba1
node backend_server.js

# Output:
# ╔════════════════════════════════════════╗
# ║  SmartHiker Web Server v1.0           ║
# ╚════════════════════════════════════════╝
#
# 🌐 Dashboard:     http://localhost:3000
# 📡 WebSocket:     ws://localhost:3000
```

#### **Step 3: Open Dashboard**
Buka browser: **http://localhost:3000**

✅ **Kelebihan:** Real-time WebSocket, multi-hiker, history data  
✅ **Production-ready:**

---

## 🔌 Integrasi dengan ESP32 & ESP8266

### **Langkah 1: Cari IP Address Laptop Anda**

#### **Windows:**
```powershell
ipconfig
# Cari IPv4 Address (biasanya 192.168.x.x atau 172.16.x.x)
```

#### **MacOS/Linux:**
```bash
ifconfig
# Cari inet address
```

Contoh: `192.168.1.100`

---

### **Langkah 2: Update ESP8266 Gateway Code**

Edit file [Proyek_Gateway_ESP8266/src/main_updated.cpp](../src/Proyek_Gateway_ESP8266/src/main_updated.cpp):

```cpp
// ========== BACKEND SERVER CONFIG ==========
#define BACKEND_HOST "192.168.1.100"   // ← GANTI IP LAPTOP ANDA
#define BACKEND_PORT 3000
#define BACKEND_API "/api/hiker-data"

// ========== FUNGSI SEND TO BACKEND ==========
void sendToBackend(String hikerName, float lat, float lon, float alt, float temp, float hum) {
  if (!WiFi.isConnected()) {
    Serial.println("❌ WiFi not connected");
    return;
  }

  HTTPClient http;
  String url = "http://" + String(BACKEND_HOST) + ":" + String(BACKEND_PORT) + String(BACKEND_API);
  
  // Buat JSON payload
  String jsonPayload = "{";
  jsonPayload += "\"hikerName\":\"" + hikerName + "\",";
  jsonPayload += "\"latitude\":" + String(lat, 6) + ",";
  jsonPayload += "\"longitude\":" + String(lon, 6) + ",";
  jsonPayload += "\"altitude\":" + String(alt, 2) + ",";
  jsonPayload += "\"temperature\":" + String(temp, 2) + ",";
  jsonPayload += "\"humidity\":" + String(hum, 2);
  jsonPayload += "}";
  
  Serial.println("📤 Sending: " + jsonPayload);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    Serial.println("✅ Response code: " + String(httpResponseCode));
  } else {
    Serial.println("❌ Error: " + String(httpResponseCode));
  }
  
  http.end();
}

// Panggil di loop():
void loop() {
  // ... sensor reading code ...
  
  // Setiap 5 detik, kirim ke backend
  if (millis() - lastBackendSend >= 5000) {
    sendToBackend("bella", currentLat, currentLon, altitude, temperature, humidity);
    lastBackendSend = millis();
  }
}
```

---

### **Langkah 3: Upload ke ESP8266**

```bash
# Buka PlatformIO terminal di VS Code
cd C:\IOT\coba1\src\Proyek_Gateway_ESP8266
platformio run --target upload
```

---

### **Langkah 4: Verify Connection**

Buka Serial Monitor ESP8266 (115200 baud):
```
✓ WiFi connected to SSID
📤 Sending: {"hikerName":"bella",...}
✅ Response code: 200
```

Lihat di dashboard:
- Marker pendaki muncul di map
- Data real-time ter-update
- Elevation chart berubah

---

## 🧪 Testing Tanpa Hardware

### **Simulate Data Menggunakan curl:**

```bash
# Kirim data dummy dari terminal
curl -X POST http://localhost:3000/api/hiker-data \
  -H "Content-Type: application/json" \
  -d '{"hikerName":"bella","latitude":-7.50,"longitude":110.50,"altitude":500,"temperature":25,"humidity":60}'

# Check dashboard - data harus muncul!
```

---

## 🛠️ Troubleshooting

| Error | Solusi |
|-------|--------|
| `npm: command not found` | Install Node.js dari nodejs.org |
| `Port 3000 already in use` | `lsof -i :3000` (Mac/Linux) atau gunakan port lain |
| `Cannot GET /` | Pastikan file berada di `src/Web_Dashboard/` |
| `WebSocket connection failed` | Cek firewall, IP address salah, atau backend tidak jalan |
| `CORS error` | Backend sudah handle CORS, reload page |

---

## 📊 API Reference

| Method | Endpoint | Deskripsi |
|--------|----------|-----------|
| GET | `/` | Dashboard HTML |
| POST | `/api/hiker-data` | Terima data GPS dari gateway |
| GET | `/api/all-hikers` | Ambil semua pendaki |
| GET | `/api/hiker/:name` | Ambil data pendaki spesifik |
| GET | `/api/hiker/:name/path` | Ambil history path |
| WS | `/` | WebSocket real-time updates |

---

## 🎨 Customization

Edit [WEB_SETUP.md](../WEB_SETUP.md) untuk:
- Deploy ke cloud
- Setup database
- Add authentication
- Custom styling

---

**Selesai! 🎉 Dashboard sudah ready!**

Butuh bantuan? Buka Developer Console (F12) untuk debug info.
