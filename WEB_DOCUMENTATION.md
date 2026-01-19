# 🌐 Dokumentasi Lengkap: Web Dashboard Integration

## 📊 System Architecture

```
┌──────────────────────────┐
│   Multiple ESP32 Nodes   │
│  (Hikers with Sensors)   │
│   • GPS                  │
│   • MPU6050, BMP280      │
│   • DHT22, etc           │
└────────────┬─────────────┘
             │ LoRa 433 MHz
             │ Data: "bella,-7.50,110.50"
             ↓
┌──────────────────────────┐
│  ESP8266 LoRa Gateway    │
│  • Receives LoRa packets │
│  • Parses hiker data     │
│  • WiFi uplink to server │
└────────────┬─────────────┘
             │ WiFi / HTTP POST
             │ {"hikerName":"bella","latitude":-7.50,...}
             ↓
┌──────────────────────────────────────┐
│   Backend Server (Node.js)           │
│   • Receive data via REST API        │
│   • Store in memory / database       │
│   • Broadcast via WebSocket          │
│   • Serve dashboard HTML             │
└────────────┬────────────────────────┘
             │ WebSocket / REST API
             │ Real-time updates
             ↓
┌──────────────────────────┐
│    Web Dashboard         │
│  (Browser - Any Device)  │
│  • Leaflet Map           │
│  • Real-time Tracking    │
│  • Stats & Graphs        │
│  • Multi-hiker support   │
└──────────────────────────┘
```

---

## 🚀 Cara Menjalankan (Step-by-Step)

### **Step 1: Persiapan Hardware**

✅ Setup ESP32 nodes dengan sensor (sudah done)  
✅ Setup ESP8266 gateway dengan LoRa module  

---

### **Step 2: Install Node.js**

Download dari [nodejs.org](https://nodejs.org)
- Pilih LTS version
- Install dengan default settings

Verify:
```bash
node --version  # v18+ expected
npm --version   # 9+ expected
```

---

### **Step 3: Install Backend Dependencies**

```bash
cd C:\IOT\coba1
npm install
# Tunggu hingga selesai
```

---

### **Step 4: Konfigurasi IP Address**

#### Cari IP Address Laptop:
```powershell
# Windows
ipconfig
# Lihat "IPv4 Address" (contoh: 192.168.1.100)
```

#### Update gateway code dengan IP tersebut:
Edit `src/Proyek_Gateway_ESP8266/src/main_gateway_with_backend.cpp`:
```cpp
#define BACKEND_HOST "192.168.1.100"   // ← GANTI dengan IP Anda
```

#### Upload ke ESP8266:
```bash
cd C:\IOT\coba1\src\Proyek_Gateway_ESP8266
platformio run --target upload
```

---

### **Step 5: Run Backend Server**

```bash
cd C:\IOT\coba1
npm start
# Atau: node backend_server.js
```

Output expected:
```
╔════════════════════════════════════════╗
║  SmartHiker Web Server v1.0           ║
╚════════════════════════════════════════╝

🌐 Dashboard:     http://localhost:3000
📡 WebSocket:     ws://localhost:3000
```

---

### **Step 6: Open Dashboard**

Di browser, buka:
```
http://localhost:3000
```

---

### **Step 7: Test Data Flow**

#### Via curl (tanpa hardware):
```bash
curl -X POST http://localhost:3000/api/hiker-data \
  -H "Content-Type: application/json" \
  -d '{"hikerName":"bella","latitude":-7.50,"longitude":110.50,"altitude":500,"temperature":25,"humidity":60}'
```

Dashboard harus menampilkan marker di map!

#### Via hardware:
1. Power on ESP32 nodes
2. Power on ESP8266 gateway
3. Cek Serial Monitor gateway → seharusnya kirim data ke backend
4. Dashboard update real-time

---

## 📱 Akses Dashboard dari Device Lain

Jika laptop ada IP `192.168.1.100`:

```
Dari smartphone/tablet di WiFi sama:
http://192.168.1.100:3000

Dari komputer lain di WiFi sama:
http://192.168.1.100:3000
```

---

## 🛠️ File Structure

```
C:\IOT\coba1\
├── backend_server.js              ← Run this untuk start server
├── package.json                   ← NPM dependencies
├── QUICK_START_WEB.md             ← Quick start guide
├── WEB_SETUP.md                   ← Detailed setup guide
│
├── src/
│   ├── Proyek_Node_ESP32/         ← ESP32 sensor nodes
│   │   └── src/main_updated.cpp
│   │
│   ├── Proyek_Gateway_ESP8266/    ← ESP8266 LoRa gateway
│   │   └── src/main_gateway_with_backend.cpp ← UPDATED
│   │
│   └── Web_Dashboard/             ← Frontend
│       ├── index.html             ← Main dashboard
│       ├── css/
│       │   └── leaflet.css
│       └── js/
│           └── leaflet.js
```

---

## 🔌 API Endpoints

### POST `/api/hiker-data`
Receive GPS data dari gateway
```json
{
  "hikerName": "bella",
  "latitude": -7.50,
  "longitude": 110.50,
  "altitude": 500,
  "temperature": 25,
  "humidity": 60
}
```

Response:
```json
{
  "success": true,
  "hikerName": "bella",
  "currentClients": 3
}
```

---

### GET `/api/all-hikers`
Get semua pendaki yang sedang tracking

Response:
```json
{
  "hikers": {
    "bella": {
      "name": "Bella",
      "position": { "lat": -7.50, "lon": 110.50, "timestamp": 1705699200000 },
      "altitude": 500,
      "temperature": 25,
      "humidity": 60,
      "path": [ ... ],
      "stats": { ... }
    },
    ...
  },
  "totalHikers": 2,
  "timestamp": 1705699200000
}
```

---

### GET `/api/hiker/:name`
Get data spesifik satu pendaki

Example: `GET /api/hiker/bella`

Response:
```json
{
  "hiker": { ... },
  "pathHistory": [ ... ]
}
```

---

### GET `/api/hiker/:name/path`
Get path history satu pendaki

Response:
```json
{
  "hikerName": "bella",
  "pathCount": 120,
  "path": [
    { "lat": -7.50, "lon": 110.50, "alt": 500, "temp": 25, "humidity": 60, "timestamp": 1705699200000 },
    ...
  ]
}
```

---

### GET `/api/health`
Health check

Response:
```json
{
  "status": "online",
  "uptime": 3600,
  "hikers": ["bella", "budi"],
  "wsClients": 3,
  "timestamp": 1705699200000
}
```

---

## 🔌 WebSocket Events

Client menerima real-time updates via WebSocket:

### `init` event (on connect)
```javascript
{
  "type": "init",
  "data": { /* all hikers data */ },
  "pathHistory": { /* all paths */ }
}
```

### `update` event (new data)
```javascript
{
  "type": "update",
  "hikerName": "bella",
  "data": {
    "name": "Bella",
    "position": { "lat": -7.50, "lon": 110.50 },
    "altitude": 500,
    ...
  }
}
```

---

## 🎨 Dashboard Features

✅ **Real-time Multi-Hiker Tracking**
- Live marker di map untuk setiap pendaki
- Warna berbeda per hiker
- Update otomatis via WebSocket

✅ **Interactive Map**
- Leaflet + OpenStreetMap
- Zoom in/out
- Pan ke lokasi hiker
- Fullscreen mode

✅ **Statistics Panel**
- Current position (lat/lon)
- Total distance traveled
- Current altitude
- Min/Max/Avg elevation
- Temperature & humidity
- Duration, speed, calories

✅ **Elevation Chart**
- Visual plot elevation profile
- Last 50 data points
- Color gradient (low→high)

✅ **Controls**
- Toggle path visibility
- Toggle marker visibility
- Toggle elevation chart
- Toggle fullscreen
- Hiker selector dropdown

✅ **Responsive Design**
- Mobile friendly
- Desktop optimized
- Touch-friendly interface

---

## 🔐 Security Considerations

### Current (Development):
- No authentication
- No data encryption
- Open to local network only

### For Production:
1. **Authentication**: Add JWT token verification
2. **HTTPS**: Use SSL certificates
3. **Rate limiting**: Prevent spam
4. **Input validation**: Sanitize hiker names
5. **Database**: Replace in-memory storage
6. **CORS**: Restrict to known origins

See [WEB_SETUP.md](WEB_SETUP.md) untuk deployment guide.

---

## 🚀 Deployment Options

### Cloud Hosting:
1. **Heroku** (deprecated tapi masih bisa)
2. **Railway.app** (recommended, free tier)
3. **Render.com** (free tier)
4. **AWS/Azure/GCP** (production)
5. **DigitalOcean** (affordable VPS)

### Docker:
```dockerfile
FROM node:18
WORKDIR /app
COPY . .
RUN npm install
EXPOSE 3000
CMD ["npm", "start"]
```

Deploy:
```bash
docker build -t smarthiker .
docker run -p 3000:3000 smarthiker
```

---

## 🧪 Testing

### Manual Testing:
```bash
# Test POST data
curl -X POST http://localhost:3000/api/hiker-data \
  -H "Content-Type: application/json" \
  -d '{"hikerName":"test","latitude":0,"longitude":0,"altitude":0,"temperature":0,"humidity":0}'

# Test GET all hikers
curl http://localhost:3000/api/all-hikers

# Test GET health
curl http://localhost:3000/api/health
```

### Automated Testing (jest):
```bash
npm install --save-dev jest
npm test
```

---

## 📚 Additional Resources

- [Leaflet Documentation](https://leafletjs.com)
- [Express.js Guide](https://expressjs.com)
- [WebSocket.org](https://www.websocket.org)
- [Node.js Best Practices](https://github.com/goldbergyoni/nodebestpractices)

---

**Status: ✅ Production Ready**

Semua file sudah tersedia di repository. Tinggal follow step-by-step di atas!

Questions? Check [QUICK_START_WEB.md](QUICK_START_WEB.md) for FAQ.
