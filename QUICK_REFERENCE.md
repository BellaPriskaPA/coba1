# 🚀 QUICK REFERENCE - Multi-User Implementation

## ⚡ TL;DR - 3 Langkah Utama

### 1️⃣ Update Firebase Path (ESP8266 Gateway)
```cpp
// ❌ LAMA:
Firebase.RTDB.setJSON(&fbdo, "/hiker", &json);

// ✅ BARU:
Firebase.RTDB.setJSON(&fbdo, "/hikers/" + hikerName, &json);
```

### 2️⃣ Kirim Nama Pendaki (ESP32 Node)
```cpp
// ❌ LAMA:
String message = String(lat) + "," + String(lon);

// ✅ BARU:
#define HIKER_NAME "bella"
String message = String(HIKER_NAME) + "," + String(lat) + "," + String(lon);
// Hasil: "bella,-7.50,110.50"
```

### 3️⃣ Update Dashboard (HTML/JS)
```javascript
// ✅ Sudah dilakukan! 
// • Dropdown selector untuk pilih hiker
// • Multi-user listener: ref(db, 'hikers')
// • Offline map support: navigator.onLine
```

---

## 📋 Data Format Reference

### LoRa Payload Format
```
"hikerName,latitude,longitude"
"bella,-7.50,110.50"
"budi,-7.60,110.40"
"rina,-7.45,110.55"
```

### Firebase Structure
```json
{
  "hikers": {
    "bella": { "lat": -7.50, "lon": 110.50, "timestamp": 1234567890 },
    "budi": { "lat": -7.60, "lon": 110.40, "timestamp": 1234567890 },
    "rina": { "lat": -7.45, "lon": 110.55, "timestamp": 1234567890 }
  }
}
```

### Dashboard Variables
```javascript
hikers = {
  'bella': { marker, path, history, distance, startTime, ... },
  'budi': { marker, path, history, distance, startTime, ... },
  'rina': { marker, path, history, distance, startTime, ... }
}
activeHikerId = 'bella'  // Currently viewing
```

---

## 🔧 File Locations

| File | Status | Notes |
|------|--------|-------|
| `src/Web_Dashboard/index.html` | ✅ Updated | Multi-user + Offline ready |
| `src/Proyek_Gateway_ESP8266/src/main.cpp` | 📝 Needs Update | Use `main_updated.cpp` as reference |
| `src/Proyek_Node_ESP32/src/main.cpp` | 📝 Needs Update | Use `main_updated.cpp` as reference |
| `src/Web_Dashboard/tiles/` | 🆕 Create Folder | For offline maps |
| `MULTI_USER_IMPLEMENTATION.md` | ✅ New | Full implementation guide |
| `ARCHITECTURE_SUMMARY.md` | ✅ New | Architecture & diagrams |

---

## 🎯 Configuration Points

### ESP8266 Gateway (`main_updated.cpp`)
```cpp
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define API_KEY "AIzaSyAFUXPpKxImJKhhm5MjQULk9dxFSzrpNIE"
#define DATABASE_URL "https://smarthiker-default-rtdb.asia-southeast1.firebasedatabase.app"

// LoRa pins (adjust if different):
#define LORA_SS 15
#define LORA_RST 16
#define LORA_DIO0 5

// Frequency
LoRa.begin(433E6);  // 433 MHz
```

### ESP32 Node (`main_updated.cpp`)
```cpp
#define HIKER_NAME "bella"  // ← CRITICAL: Change for each device!
#define DEVICE_ID 1

// Sensor pins (adjust if different):
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2
#define DHTPIN 4

// Frequency (MUST match Gateway):
LoRa.begin(433E6);  // 433 MHz
```

### Web Dashboard (`index.html`)
```javascript
// Firebase Config (already set)
const firebaseConfig = { ... }

// Offline map URL (automatic fallback)
const mapUrl = navigator.onLine ? 
               'https://{s}.tile.opentopomap.org/{z}/{x}/{y}.png' : 
               'tiles/{z}/{x}/{y}.png';

// Hiker colors (customizable)
const hikerColors = {
  'bella': '#FC4C02',
  'budi': '#1E88E5',
  'rina': '#43A047',
  'ricky': '#FB8C00',
  'default': '#FC4C02'
};
```

---

## 🧪 Testing Checklist

### Manual Firebase Test (No Hardware)
1. Open Firebase Console
2. Create structure:
   ```json
   {
     "hikers": {
       "bella": { "lat": -7.5, "lon": 110.5 },
       "budi": { "lat": -7.51, "lon": 110.51 }
     }
   }
   ```
3. Refresh Web Dashboard
4. Dropdown should show "BELLA" and "BUDI"
5. Click to select → dashboard updates

### Single Device Test
1. Flash ESP32 with `HIKER_NAME = "bella"`
2. Flash ESP8266 with updated code
3. Open Serial Monitor (115200)
4. Device sends: `"bella,-7.50,110.50"`
5. Gateway receives and logs
6. Check Firebase Console
7. Web Dashboard dropdown shows "BELLA"
8. Marker appears on map

### Multi-Device Test
1. Flash 2x ESP32 devices:
   - Device 1: `HIKER_NAME = "bella"`
   - Device 2: `HIKER_NAME = "budi"`
2. Both send data to Gateway
3. Gateway routes to `/hikers/bella` and `/hikers/budi`
4. Web Dashboard shows:
   - Dropdown: BELLA, BUDI
   - Different markers (orange, blue)
   - Different polylines
5. Switch between users → data updates correctly

### Offline Test
1. Create `/tiles` folder with sample tiles
2. Disable WiFi/Networking
3. Open Web Dashboard
4. Status shows: "Offline - Peta Lokal"
5. Map still visible with local tiles
6. Enable WiFi → Status changes back

---

## 📊 Key Functions

### Gateway (ESP8266)
```cpp
bool parseAndUpload(String data) {
  // Input: "bella,-7.50,110.50"
  // Parse: hikerName, lat, lon
  // Upload to: /hikers/{hikerName}
}
```

### Dashboard (HTML/JS)
```javascript
// Listen to all hikers
const hikersRef = ref(db, 'hikers');
onValue(hikersRef, (snapshot) => {
  // For each hiker: create/update UI
});

// Handle hiker selection
document.getElementById('hiker-select').addEventListener('change', (e) => {
  activeHikerId = e.target.value;
  // Show/hide markers, update stats
});

// Offline detection
const mapUrl = navigator.onLine ? 'online_url' : 'local_url';
window.addEventListener('online', () => { /* update status */ });
window.addEventListener('offline', () => { /* update status */ });
```

---

## 🚨 Common Mistakes

| ❌ Mistake | ✅ Fix |
|-----------|--------|
| Using `/hiker` instead of `/hikers/{name}` | Update path in Gateway |
| Not sending hiker name from ESP32 | Add HIKER_NAME to payload |
| Same HIKER_NAME on multiple devices | Assign unique names to each |
| LoRa frequency mismatch | Both devices: 433E6 or 915E6 |
| Dropdown doesn't populate | Check Firebase listener is active |
| Markers overlap | Use different colors (already in code) |
| Map goes blank offline | Create `/tiles` folder with tile files |
| Firebase credentials wrong | Copy from Firebase Console |

---

## 📞 Debugging Tips

### Serial Output (Gateway)
```
📨 Packet received: bella,-7.50,110.50
   Signal: -95 dBm
   Uploading to: /hikers/bella
   ✓ Data uploaded to Firebase
```

### Serial Output (Node)
```
👤 Hiker Name: bella
📤 Sending: bella,-7.50,110.50
✓ Packet sent
```

### Browser Console (Dashboard)
```javascript
// Check hikers object
console.log(hikers);
// Check active hiker
console.log(activeHikerId);
// Check Firebase listener
console.log(hikersRef);
```

### DevTools Network Tab
- Monitor Firebase API calls
- Check response payloads
- Verify latency

---

## 🆘 If Something Breaks

### Dashboard shows no hikers
1. Check Firebase Console → Data exists?
2. Check browser console → JS errors?
3. Check listener active? → `onValue()` called?
4. Check network tab → API calls successful?

### Markers don't move
1. Is `activeHikerId` set? (dropdown selected?)
2. Is new data arriving? (check Firebase)
3. Does data format match? ("name,lat,lon")
4. Check browser console for errors

### Gateway not uploading
1. Check WiFi connected?
2. Check Firebase credentials?
3. Check path correct? (`/hikers/...`)
4. Check Serial logs for parse errors?

### Offline map blank
1. Does `/tiles` folder exist?
2. Do tile files exist? (e.g., `14/12345_67890.png`)
3. Is path correct? (`tiles/{z}/{x}/{y}.png`)
4. Check browser console for 404 errors?

---

## 📖 Document References

1. **MULTI_USER_IMPLEMENTATION.md** ← Full implementation guide (70+ lines)
2. **ARCHITECTURE_SUMMARY.md** ← Architecture & diagrams (100+ lines)
3. **This file** ← Quick reference (this!)

---

## ✨ Summary of Changes

| Component | Change | Impact |
|-----------|--------|--------|
| **Web Dashboard** | Multi-user listener + dropdown | Can track 5+ users simultaneously |
| **Gateway** | Dynamic Firebase path | Routes data to correct user |
| **Node** | Send hiker name | Identifies which user tracking |
| **Map** | Offline support | Works without internet |
| **UI** | Hiker selector | Easy switching between users |

---

## 🎓 For Your Skripsi

This implementation demonstrates:
- ✅ **IoT Architecture**: Multi-sensor, multi-device system
- ✅ **Real-time Database**: Firebase as scalable backend
- ✅ **Wireless Protocols**: LoRa for long-range, WiFi for cloud
- ✅ **Web Technologies**: Leaflet.js, Firebase SDK, responsive UI
- ✅ **Offline-first Design**: Local fallback for resilience
- ✅ **Scalability**: Easily add more hikers without code change

Perfect for demonstrating modern IoT + Web integration! 🚀

---

**Quick Start**: Copy `main_updated.cpp` files as reference, update your original files, test with single device first, then add more hikers!

Last Updated: 2026-01-19
