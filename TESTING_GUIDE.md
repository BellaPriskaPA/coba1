# 🧪 Testing & Troubleshooting Guide

## Phase 1: Local Testing (No Hardware)

### Test 1.1: Firebase Manual Data Entry

**Tujuan**: Verifikasi dashboard dapat membaca multi-user data

**Steps**:
1. Buka Firebase Console → smarthiker project
2. Klik "Realtime Database"
3. Hapus data lama atau buat di struktur baru
4. Input data manually:
   ```json
   {
     "hikers": {
       "bella": {
         "lat": -7.5,
         "lon": 110.5,
         "timestamp": 1234567890
       },
       "budi": {
         "lat": -7.51,
         "lon": 110.51,
         "timestamp": 1234567890
       }
     }
   }
   ```
5. Buka Web Dashboard di browser
6. Tunggu 2-3 detik

**Expected Results**:
- [ ] Dropdown menampilkan "BELLA" dan "BUDI"
- [ ] Marker muncul di peta (Bella di [-7.5, 110.5])
- [ ] Dropdown default memilih BELLA (atau hiker pertama)
- [ ] Dashboard menampilkan: "MOUNTAIN TRACKER" → bisa jadi "BELLA"
- [ ] Posisi menunjukkan: "-7.50000, 110.50000"
- [ ] Status koneksi: "Aktif" (🟢 hijau)

**Troubleshooting**:
```
Dropdown kosong?
  ❌ Firebase listener tidak aktif
  ✅ Buka DevTools (F12) → Console
  ✅ Cek error messages
  ✅ Pastikan credentials benar di index.html

Marker tidak muncul?
  ❌ activeHikerId belum di-set
  ✅ Pilih hiker dari dropdown manually
  ✅ Cek console untuk JS errors
  
Dashboard tidak update?
  ❌ Data belum sync
  ✅ Reload halaman (Ctrl+R)
  ✅ Cek network tab → Firebase API calls
```

---

### Test 1.2: Data Update Test

**Tujuan**: Verifikasi real-time update saat data berubah

**Steps**:
1. Buka Firebase Console di satu tab
2. Buka Dashboard di tab lain
3. Di Firebase Console, ubah data BELLA:
   ```json
   "bella": {
     "lat": -7.51,
     "lon": 110.51,
     "timestamp": 1234567891
   }
   ```
4. Amati Dashboard

**Expected Results**:
- [ ] Marker bergerak ke posisi baru ([-7.51, 110.51])
- [ ] Posisi di dashboard update: "-7.51000, 110.51000"
- [ ] Path (polyline) bertambah 1 segment baru
- [ ] Jarak bertambah (~100m)
- [ ] Update terjadi dalam 1-3 detik

**Troubleshooting**:
```
Marker tidak bergerak?
  ❌ Path tidak ada, check calculateDistance function
  ✅ Buka console → cek hikers object
  ✅ Verify marker telah di-set ke map
  
Jarak tidak bertambah?
  ❌ calculateDistance menghasilkan 0
  ✅ Check if lastPosition di-update
  ✅ Verify distance > 0.001 km threshold
```

---

## Phase 2: Single Device Testing

### Test 2.1: ESP8266 Gateway Serial Output

**Tujuan**: Verifikasi gateway dapat menerima dan parse LoRa packet

**Hardware**:
- 1x ESP8266 (Gateway)
- 1x ESP32 (Node)
- LoRa module untuk keduanya

**Steps**:
1. Flash ESP8266 dengan `main_updated.cpp`
2. Buka Serial Monitor (COM port, 115200 baud)
3. Pastikan "Newline" di dropdown
4. Flash ESP32 dengan `main_updated.cpp` (HIKER_NAME = "bella")
5. Tunggu data mulai dikirim

**Expected Serial Output**:
```
╔══════════════════════════════════════╗
║  SmartHiker Gateway (ESP8266) v2.0   ║
║  Multi-User LoRa to Firebase         ║
╚══════════════════════════════════════╝

📶 Connecting to WiFi: . . . . . . . . ✓ Connected!
   IP: 192.168.1.100
🔥 Firebase initialized
📡 Initializing LoRa...
✓ LoRa ready (433 MHz, SF7)

═══════════════════════════════════════
Waiting for LoRa packets...

📨 Packet received: bella,-7.50000,110.50000
   Signal: -92 dBm
   Uploading to: /hikers/bella
   📊 bella [-7.5000, 110.5000]
✓ Data uploaded to Firebase

📨 Packet received: bella,-7.50100,110.50100
   ...
```

**Troubleshooting**:
```
Serial kosong (no output)?
  ❌ Baud rate salah
  ✅ Set ke 115200
  ❌ Wrong COM port
  ✅ Check Device Manager
  
"Format error"?
  ❌ ESP32 mengirim format salah
  ✅ Verify: "name,lat,lon"
  ✅ Check HIKER_NAME tidak ada spasi
  
"Firebase error"?
  ❌ WiFi tidak terhubung
  ✅ Verify SSID & password
  ❌ Firebase credentials salah
  ✅ Copy API_KEY & DATABASE_URL dari Console
```

---

### Test 2.2: Firebase Console Verification

**Tujuan**: Verifikasi data berhasil di-upload ke Firebase

**Steps**:
1. Buka Firebase Console
2. Klik "Realtime Database"
3. Amati struktur data real-time

**Expected Structure**:
```json
{
  "hikers": {
    "bella": {
      "lat": -7.50001,
      "lon": 110.50001,
      "timestamp": 1234567890,
      "rssi": -92
    }
  }
}
```

**Troubleshooting**:
```
/hikers/bella tidak ada?
  ❌ Gateway tidak mengirim data
  ✅ Check Gateway serial output
  ❌ Path salah di code
  ✅ Verify path: "/hikers/" + hikerName
  
Data tidak update?
  ❌ LoRa packet loss
  ✅ Check signal strength (RSSI)
  ✅ Reduce distance antara devices
  ❌ Firebase throttled
  ✅ Check billing & quotas
```

---

### Test 2.3: Web Dashboard Single User

**Tujuan**: Verifikasi dashboard bekerja dengan 1 pendaki

**Steps**:
1. Pastikan BELLA data ada di Firebase (dari Test 2.2)
2. Buka Web Dashboard di browser
3. Amati UI elements

**Checklist**:
- [ ] Dropdown menampilkan "BELLA"
- [ ] Dropdown default value = "BELLA"
- [ ] Badge "AKTIF" tampil di sebelah dropdown
- [ ] Map menampilkan marker orange (Bella)
- [ ] Map menampilkan polyline orange (path)
- [ ] Dashboard stats update (jarak, waktu, elevasi)
- [ ] Posisi menunjukkan koordinat BELLA
- [ ] Toggle buttons berfungsi:
  - [ ] Path visibility toggle
  - [ ] Elevation panel toggle
  - [ ] Marker visibility toggle

**Expected Visual**:
```
┌─────────────────────────────────────┐
│ BELLA TRACKER                       │
│ Pilih Pendaki: [BELLA ▼] [AKTIF]   │
│                                     │
│ ┌─────┬───────┬───────┬──────┐    │
│ │2.5km│02:15:30│3.2km/j│+125m │    │
│ └─────┴───────┴───────┴──────┘    │
│                                     │
│ [Map with BELLA marker & path]      │
│                                     │
│ Status: 🟢 Aktif                   │
│ Posisi: -7.50000, 110.50000        │
└─────────────────────────────────────┘
```

**Troubleshooting**:
```
Dropdown empty?
  ✅ Check Firebase data exists
  ✅ Check listener: ref(db, 'hikers')
  
Dashboard blank?
  ❌ activeHikerId = null
  ✅ Manually select from dropdown
  
Polyline missing?
  ❌ positions array kosong
  ✅ Send multiple packets (need 2+ points)
  
Stats tidak update?
  ❌ updateDashboardStats() tidak dipanggil
  ✅ Check interval timer berjalan
```

---

## Phase 3: Multi-Device Testing

### Test 3.1: Two Device Setup

**Hardware**:
- 1x ESP8266 (Gateway)
- 2x ESP32 (Node):
  - Node 1: HIKER_NAME = "bella"
  - Node 2: HIKER_NAME = "budi"

**Steps**:
1. Flash ESP32 Node 1 dengan HIKER_NAME = "bella"
2. Flash ESP32 Node 2 dengan HIKER_NAME = "budi"
3. Pastikan keduanya di-powered
4. Amati Serial output Gateway (mix dari bella & budi packets)
5. Check Firebase Console → /hikers/bella dan /hikers/budi

**Expected Results**:
```
Gateway Serial Monitor akan tampil:
📨 Packet received: bella,-7.50,110.50
   ✓ Data uploaded to Firebase
📨 Packet received: budi,-7.60,110.40
   ✓ Data uploaded to Firebase
📨 Packet received: bella,-7.51,110.51
   ✓ Data uploaded to Firebase
```

**Firebase Console**:
```json
{
  "hikers": {
    "bella": {
      "lat": -7.51,
      "lon": 110.51
    },
    "budi": {
      "lat": -7.60,
      "lon": 110.40
    }
  }
}
```

---

### Test 3.2: Dashboard Multi-User Display

**Tujuan**: Verifikasi dashboard menampilkan 2+ pendaki

**Steps**:
1. Buka Dashboard
2. Tunggu 2-3 detik

**Expected Results**:
- [ ] Dropdown menampilkan "BELLA" dan "BUDI"
- [ ] Dropdown default select BELLA
- [ ] Map menampilkan 2 markers:
  - [ ] BELLA marker (orange) di [-7.51, 110.51]
  - [ ] BUDI marker (biru) di [-7.60, 110.40]
- [ ] 2 polylines visible (orange & biru)
- [ ] Dashboard stats menampilkan BELLA data

**User Interaction Test**:
1. Klik dropdown → pilih "BUDI"
   - [ ] BELLA marker hilang
   - [ ] BUDI marker tetap/fokus
   - [ ] Polyline berubah ke biru
   - [ ] Dashboard stats berubah ke BUDI
   - [ ] Map pan ke BUDI location

2. Klik dropdown → kembali ke "BELLA"
   - [ ] BELLA marker kembali
   - [ ] BUDI marker hilang
   - [ ] Polyline berubah ke orange
   - [ ] Dashboard stats kembali ke BELLA
   - [ ] Map pan ke BELLA location

**Visual Expected**:
```
┌────────────────────────────────────────┐
│ BELLA TRACKER                          │
│ Pilih Pendaki: [BELLA ▼] [AKTIF]      │
│ ┌──────────────────────┐               │
│ │ -- Tunggu Pendaki -- │ (before data) │
│ │ BELLA ✓ (current)    │               │
│ │ BUDI                 │               │
│ └──────────────────────┘               │
│                                        │
│ [Map showing BELLA marker & path]      │
│ (BUDI marker not visible)              │
│                                        │
│ Stats:                                 │
│ • BELLA's distance, time, elevation    │
└────────────────────────────────────────┘

Switch to BUDI:
│ [Map showing BUDI marker & path]       │
│ (BELLA marker not visible)             │
│ Stats:                                 │
│ • BUDI's distance, time, elevation     │
```

**Troubleshooting**:
```
Dropdown shows only BELLA?
  ❌ BUDI data tidak di-upload
  ✅ Check Gateway serial → BUDI packet received?
  ✅ Check Firebase → /hikers/budi exists?
  
Semua pendaki marker terlihat?
  ❌ activeHikerId logic salah
  ✅ Only selected hiker's marker should show
  
Polyline warna tidak berubah?
  ❌ hikerColors mapping salah
  ✅ Verify warna untuk 'bella' vs 'budi'
  
Stats tidak switch?
  ❌ updateDashboardStats() tidak dipanggil saat dropdown change
  ✅ Check event listener: hiker-select.addEventListener
```

---

## Phase 4: Offline Testing

### Test 4.1: Offline Map Setup

**Tujuan**: Verifikasi dashboard bekerja tanpa internet

**Prerequisites**:
- Folder `/tiles` dengan struktur:
  ```
  Web_Dashboard/tiles/
  ├── 14/
  │   └── 8250_5241.png
  │   └── 8251_5241.png
  │   └── ... (banyak files)
  ├── 15/
  │   └── ... (tile files)
  └── 16/
      └── ... (tile files)
  ```

**Steps untuk Download Tiles** (gunakan QGIS):
1. Install QGIS
2. Plugins → Manage and Install → cari "Slippy Map Tileset Downloader"
3. Plugins → Slippy Map Tileset Downloader
4. Input:
   - Min zoom: 14
   - Max zoom: 16
   - Bounds: Yogyakarta area
   - Tile service: OpenTopoMap
5. Download ke folder `Web_Dashboard/tiles/`

**Alternative - Python Script**:
```python
# download_tiles.py
import os
import urllib.request
import math

def download_tiles(lat, lon, zoom_range, output_dir):
    for zoom in zoom_range:
        os.makedirs(f"{output_dir}/{zoom}", exist_ok=True)
        n = 2.0 ** zoom
        xtile = int((lon + 180) / 360 * n)
        ytile = int((1.0 - math.log(math.tan(math.radians(lat)) + 1.0 / math.cos(math.radians(lat))) / math.pi) / 2.0 * n)
        
        for x in range(xtile - 10, xtile + 10):
            for y in range(ytile - 10, ytile + 10):
                try:
                    url = f"https://tile.opentopomap.org/{zoom}/{x}/{y}.png"
                    path = f"{output_dir}/{zoom}/{x}_{y}.png"
                    urllib.request.urlretrieve(url, path)
                    print(f"✓ {zoom}/{x}/{y}")
                except Exception as e:
                    print(f"✗ {zoom}/{x}/{y}: {e}")

# Download untuk Yogyakarta (-7.5, 110.5), zoom 14-16
download_tiles(-7.5, 110.5, range(14, 17), "Web_Dashboard/tiles")
```

### Test 4.2: Offline Mode Test

**Steps**:
1. Buka Dashboard (dengan internet)
2. Verify tiles sudah ter-cache (buka DevTools → Application → Cache Storage)
3. Atau set WiFi ke offline mode:
   - Windows: Network Settings → Airplane Mode ON
   - Mac: System Preferences → Wi-Fi → OFF
   - Browser: DevTools → Network → Offline
4. Refresh Dashboard (Ctrl+R)

**Expected Results**:
- [ ] Map masih visible (using local tiles)
- [ ] Status indicator: "Offline - Peta Lokal" (🟠 orange)
- [ ] Marker masih tampil (dari memory)
- [ ] Dashboard stats masih visible (cached data)
- [ ] Toggle buttons tetap berfungsi
- [ ] Zoom/pan masih bekerja (untuk area dengan tiles)

**Expected Visual**:
```
Console Browser:
navigator.onLine → false

Status Panel:
"🟠 Offline - Peta Lokal"

Map:
[Tiles from /tiles/ folder]
[Marker dari last known position]
[Polyline dari tracking data in memory]
```

**Troubleshooting**:
```
Map blank offline?
  ❌ /tiles folder tidak ada
  ✅ Create folder structure
  ❌ Path salah: tiles/{z}/{x}/{y}.png
  ✅ Verify path di mapUrl variable
  
Status tidak berubah ke Offline?
  ❌ navigator.onLine tidak trigger
  ✅ Check browser console
  ✅ Verify event listeners:
     window.addEventListener('offline')
     window.addEventListener('online')
```

---

## Phase 5: Stress Testing

### Test 5.1: Rapid Data Updates

**Tujuan**: Verifikasi dashboard stabil dengan frequent updates

**Steps**:
1. Ubah `LORA_SEND_INTERVAL` di ESP32 menjadi 500ms (was 3000ms)
2. Run selama 5 menit
3. Amati browser & Firebase

**Metrics to Monitor**:
- [ ] CPU usage (Task Manager)
- [ ] Memory usage (DevTools)
- [ ] Frame rate (DevTools Performance)
- [ ] Firebase quota

**Expected**:
- CPU < 30%
- Memory < 50MB
- Frame rate > 30fps
- No error messages

---

### Test 5.2: Multiple Hikers (5+)

**Tujuan**: Verifikasi dashboard scalability

**Steps**:
1. Manually create di Firebase:
   ```json
   {
     "hikers": {
       "bella": { "lat": -7.50, "lon": 110.50 },
       "budi": { "lat": -7.51, "lon": 110.51 },
       "rina": { "lat": -7.52, "lon": 110.52 },
       "ricky": { "lat": -7.53, "lon": 110.53 },
       "sri": { "lat": -7.54, "lon": 110.54 }
     }
   }
   ```
2. Buka Dashboard
3. Cycle through dropdown
4. Monitor performance

**Expected**:
- Dropdown menampilkan 5 names
- Each selection loads in < 1s
- Map updates smoothly
- No lag or crashes

---

## Success Criteria Summary

✅ **Phase 1 Complete**: Data manual di Firebase → Dashboard displays correctly

✅ **Phase 2 Complete**: Single ESP32+ESP8266 → Data flows end-to-end

✅ **Phase 3 Complete**: Multiple hikers → Dashboard switches between users smoothly

✅ **Phase 4 Complete**: Offline → Map & data visible without internet

✅ **Phase 5 Complete**: Performance → No lag with 5+ users

---

## Final Checklist Before Deployment

- [ ] All 3 ESP32 nodes flashed with unique HIKER_NAME
- [ ] ESP8266 gateway running with updated code
- [ ] Dashboard deployed to web server or local
- [ ] `/tiles` folder created with offline maps (if needed)
- [ ] Firebase Realtime Database set to /hikers structure
- [ ] All 5 devices tested together
- [ ] Offline mode tested & working
- [ ] Error handling verified
- [ ] Performance acceptable (no lag)
- [ ] Documentation updated with actual device names

---

**Testing Duration**: 2-3 hours per phase  
**Total Time**: ~12-15 hours full testing  
**Status**: Ready to deploy after completion

Good luck with your testing! 🚀
