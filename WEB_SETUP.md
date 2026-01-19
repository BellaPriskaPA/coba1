# 🌐 Web Dashboard Setup & Deployment Guide

## Overview
Web Dashboard adalah aplikasi real-time untuk tracking lokasi para pendaki dengan peta interaktif (Leaflet + OpenStreetMap), statistik altitude/suhu, dan sistem multi-user.

---

## 🏗️ Arsitektur Sistem

```
┌─────────────────────────────────────────────────────┐
│  ESP32 NODES (Multi-Hiker)                          │
│  - Membaca: GPS, MPU6050, BMP280, DHT22             │
│  - Mengirim: "hikerName,lat,lon" via LoRa           │
└──────────────────────┬──────────────────────────────┘
                       │ LoRa 433 MHz
                       ↓
┌──────────────────────────────────────────────────────┐
│  ESP8266 GATEWAY                                    │
│  - Menerima data LoRa dari all nodes                │
│  - Parse: "bella,-7.50,110.50"                      │
│  - Kirim via WiFi/MQTT ke Backend                   │
└──────────────────────┬───────────────────────────────┘
                       │ WiFi/MQTT/HTTP
                       ↓
┌──────────────────────────────────────────────────────┐
│  BACKEND SERVER (Node.js/Python/Cloud)              │
│  - Store data di database (MongoDB/PostgreSQL)      │
│  - WebSocket API untuk real-time streaming          │
│  - REST API untuk query historical data             │
└──────────────────────┬───────────────────────────────┘
                       │ WebSocket/REST API
                       ↓
┌──────────────────────────────────────────────────────┐
│  WEB DASHBOARD (Browser)                            │
│  - File statis: index.html, css/style.css, js/app.js│
│  - Leaflet Map untuk visualisasi GPS                │
│  - Real-time update via WebSocket                   │
│  - Multi-hiker tracking & stats                     │
└──────────────────────────────────────────────────────┘
```

---

## 🚀 Setup & Running

### **Option 1: Local Web Server (Fastest)**
Untuk development/testing lokal:

#### **Windows:**
```powershell
# Jika punya Python 3:
cd C:\IOT\coba1\src\Web_Dashboard
python -m http.server 8000

# ATAU jika punya Node.js:
npx http-server -p 8000

# Buka browser:
# http://localhost:8000
```

#### **MacOS/Linux:**
```bash
cd ~/IOT/coba1/src/Web_Dashboard
python3 -m http.server 8000

# Buka browser:
# http://localhost:8000
```

---

### **Option 2: VS Code Live Server Extension**
1. Install extension: **Live Server** (Ritwick Dey)
2. Right-click `index.html` → "Open with Live Server"
3. Browser otomatis buka di `http://127.0.0.1:5500`

---

### **Option 3: Backend Integration (Recommended)**
Untuk production dengan real-time data:

#### **Step 1: Setup Backend Server**
Buat file `backend_server.js` (Node.js):

```javascript
const express = require('express');
const cors = require('cors');
const WebSocket = require('ws');
const http = require('http');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// Serve static files (dashboard)
app.use(express.static(path.join(__dirname, 'src/Web_Dashboard')));
app.use(cors());
app.use(express.json());

// In-memory hiker data (ganti dengan database di production)
let hikersData = {
  bella: {
    name: "Bella",
    lastUpdate: Date.now(),
    position: { lat: -7.50, lon: 110.50 },
    altitude: 500,
    temperature: 25,
    humidity: 60,
    path: []
  },
  budi: {
    name: "Budi",
    lastUpdate: Date.now(),
    position: { lat: -7.51, lon: 110.51 },
    altitude: 520,
    temperature: 24,
    humidity: 65,
    path: []
  }
};

// WebSocket connection untuk broadcast ke semua client
wss.on('connection', (ws) => {
  console.log('Client connected');
  
  // Send initial data
  ws.send(JSON.stringify({
    type: 'init',
    data: hikersData
  }));

  ws.on('close', () => console.log('Client disconnected'));
});

// API untuk receive data dari ESP8266 Gateway
app.post('/api/hiker-data', (req, res) => {
  const { hikerName, latitude, longitude, altitude, temperature, humidity } = req.body;
  
  if (!hikersData[hikerName]) {
    hikersData[hikerName] = {
      name: hikerName.charAt(0).toUpperCase() + hikerName.slice(1),
      path: []
    };
  }

  // Update hiker data
  hikersData[hikerName].lastUpdate = Date.now();
  hikersData[hikerName].position = { lat: latitude, lon: longitude };
  hikersData[hikerName].altitude = altitude || 0;
  hikersData[hikerName].temperature = temperature || 0;
  hikersData[hikerName].humidity = humidity || 0;
  
  // Add to path history
  hikersData[hikerName].path.push({
    lat: latitude,
    lon: longitude,
    timestamp: Date.now()
  });

  // Broadcast ke semua connected clients
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify({
        type: 'update',
        hikerName: hikerName,
        data: hikersData[hikerName]
      }));
    }
  });

  res.json({ success: true, message: 'Data received' });
});

// API untuk query historical data
app.get('/api/hiker/:name', (req, res) => {
  const hikerName = req.params.name.toLowerCase();
  res.json(hikersData[hikerName] || null);
});

app.get('/api/all-hikers', (req, res) => {
  res.json(hikersData);
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`🚀 Server running on http://localhost:${PORT}`);
  console.log(`📊 Dashboard: http://localhost:${PORT}`);
  console.log(`📡 WebSocket: ws://localhost:${PORT}`);
});
```

#### **Step 2: Install Dependencies**
```bash
npm install express cors ws
```

#### **Step 3: Update ESP8266 Gateway Code**
Gateway harus POST data ke backend:

```cpp
// Di Proyek_Gateway_ESP8266
void sendToBackend(String hikerName, float lat, float lon, float alt, float temp, float hum) {
  HTTPClient http;
  
  String url = "http://YOUR_BACKEND_IP:3000/api/hiker-data";
  String jsonData = "{\"hikerName\":\"" + hikerName + "\",\"latitude\":" + lat + 
                    ",\"longitude\":" + lon + ",\"altitude\":" + alt + 
                    ",\"temperature\":" + temp + ",\"humidity\":" + hum + "}";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonData);
  http.end();
}
```

#### **Step 4: Run Backend & Dashboard**
```bash
node backend_server.js
# Server jalan di http://localhost:3000
```

---

## 📱 Accessing Dashboard

### **Local Network:**
- Semua device di WiFi yang sama: `http://192.168.1.100:3000`

### **Public Internet (Production):**
1. Deploy ke cloud (Heroku, Railway, Render, AWS, Azure)
2. Update WebSocket URL di `index.html`:
```javascript
const wsUrl = "wss://your-production-url.com";  // Ganti domain
const apiUrl = "https://your-production-url.com/api"; // Ganti domain
```

---

## 🔧 Troubleshooting

| Issue | Solution |
|-------|----------|
| "Cannot connect to server" | Pastikan backend/server sedang berjalan |
| Map tidak load | Check internet connection, Leaflet CDN harus accessible |
| Data tidak real-time | Cek WebSocket connection, buka DevTools (F12) |
| Port 3000 sudah terpakai | Gunakan port lain: `PORT=8080 node backend_server.js` |

---

## 📊 Dashboard Features

✅ Real-time multi-hiker tracking  
✅ Interactive Leaflet map dengan OpenStreetMap  
✅ Altitude elevation chart  
✅ Temperature & humidity monitoring  
✅ Path history visualization  
✅ Hiker selector dropdown  
✅ Toggle controls (path, elevation, markers, fullscreen)  
✅ Connection status indicator  
✅ Offline mode support  

---

## 🎯 Next Steps

1. **Atur Backend Server** dengan setup di atas
2. **Update ESP8266 Gateway** untuk POST ke backend
3. **Deploy ke cloud** untuk akses public
4. **Setup database** untuk persistent storage
5. **Add authentication** untuk security

Semuanya ready! 🚀
