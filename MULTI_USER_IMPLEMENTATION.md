# 🎯 Panduan Implementasi Multi-User & Offline Mode

## 📋 Ringkasan Perubahan

Sekarang dashboard sudah mendukung:
- ✅ **Multi-User Detection** - Deteksi & tampilkan Bella, Budi, Rina, dan user lain secara bersamaan
- ✅ **Offline Map Support** - Fallback ke folder `/tiles` jika tidak ada internet
- ✅ **Hiker Selector Dropdown** - Pilih pendaki mana yang ingin dipantau
- ✅ **Individual Tracking** - Setiap pendaki punya warna, polyline, dan data terpisah
- ✅ **Online/Offline Status** - Indikator real-time status koneksi

---

## 🚀 LANGKAH 1: Update Firebase Path di Gateway (ESP8266)

### ❌ Kode Lama (Hanya untuk 1 user):
```cpp
Firebase.RTDB.setJSON(&fbdo, "/hiker", &json);
```

### ✅ Kode Baru (Multi-user):
Anda harus mengubah path menjadi `/hikers/{nama_pendaki}`.

**Contoh untuk Bella:**
```cpp
Firebase.RTDB.setJSON(&fbdo, "/hikers/bella", &json);
```

**Contoh untuk Budi:**
```cpp
Firebase.RTDB.setJSON(&fbdo, "/hikers/budi", &json);
```

### Solusi Dinamis (Rekomendasi):

Anda bisa menambahkan identifikasi pendaki dari ESP32 Node. Berikut update `main.cpp` untuk Gateway:

```cpp
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LoRa.h>
#include <SPI.h>

#define WIFI_SSID "NAMA_WIFI"
#define WIFI_PASSWORD "PASS_WIFI"
#define API_KEY "AIzaSyAFUXPpKxImJKhhm5MjQULk9dxFSzrpNIE"
#define DATABASE_URL "https://smarthiker-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Init LoRa
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
    while (LoRa.available()) data += (char)LoRa.read();
    
    // Format baru: "hikerName,lat,lon"
    // Contoh: "bella,-7.5,110.5"
    int firstComma = data.indexOf(',');
    int secondComma = data.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      String hikerName = data.substring(0, firstComma);
      String lat = data.substring(firstComma + 1, secondComma);
      String lon = data.substring(secondComma + 1);

      // Upload ke Firebase dengan path yang benar
      FirebaseJson json;
      json.set("lat", lat);
      json.set("lon", lon);
      json.set("timestamp", millis());
      
      // PENTING: Format path dengan nama pendaki
      String path = "/hikers/" + hikerName;
      Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json);
      
      Serial.println("✓ Data " + hikerName + " -> Firebase");
    }
  }
}
```

---

## 📱 LANGKAH 2: Update ESP32 Node (Kirim Data dengan Nama)

Update `src/Proyek_Node_ESP32/src/main.cpp` agar mengirim nama pendaki:

### ❌ Kode Lama:
```cpp
// Hanya kirim lat,lon
String message = String(lat) + "," + String(lon);
LoRa.beginPacket();
LoRa.print(message);
LoRa.endPacket();
```

### ✅ Kode Baru:
```cpp
// Kirim: "nama,lat,lon"
#define HIKER_NAME "bella"  // Ganti dengan nama pendaki

// Di dalam loop:
String message = String(HIKER_NAME) + "," + String(lat) + "," + String(lon);
LoRa.beginPacket();
LoRa.print(message);
LoRa.endPacket();
```

---

## 🗺️ LANGKAH 3: Setup Offline Map (Optional Tapi Penting!)

### Folder Structure:
Letakkan file tiles di folder ini:
```
Web_Dashboard/
├── index.html
├── css/
├── js/
└── tiles/           ← BUAT FOLDER INI
    ├── 14/
    │   └── ...tile files...
    ├── 15/
    │   └── ...tile files...
    └── 16/
        └── ...tile files...
```

### Cara Download Tiles (Offline Map):

1. **Gunakan QGIS (Gratis)**:
   - Install QGIS
   - Plugin → Manage and Install Plugins → cari "Slippy Map Tileset Downloader"
   - Download area Yogyakarta zoom 14-16

2. **Atau Gunakan Script Python**:
```python
import os
import urllib.request

def download_tiles(lat, lon, zoom, output_dir):
    """Download tile dari OpenTopoMap untuk area tertentu"""
    import math
    
    def lat_lon_to_tile(lat, lon, zoom):
        x = (lon + 180) / 360 * (1 << zoom)
        y = (1 - math.log(math.tan(math.radians(lat)) + 1/math.cos(math.radians(lat))) / math.pi) / 2 * (1 << zoom)
        return int(x), int(y)
    
    tx, ty = lat_lon_to_tile(lat, lon, zoom)
    
    for z in range(14, 17):
        os.makedirs(f"{output_dir}/{z}", exist_ok=True)
        for x in range(tx - 5, tx + 5):
            for y in range(ty - 5, ty + 5):
                url = f"https://tile.opentopomap.org/{z}/{x}/{y}.png"
                filepath = f"{output_dir}/{z}/{x}_{y}.png"
                try:
                    urllib.request.urlretrieve(url, filepath)
                    print(f"✓ Downloaded {z}/{x}/{y}")
                except:
                    pass

# Download untuk area Yogyakarta (-7.5, 110.5)
download_tiles(-7.5, 110.5, 15, "Web_Dashboard/tiles")
```

3. **Atau Manual - Download dari Online Tile Repository**:
   - Kunjungi: https://tiles.opentopomap.org/
   - Jangan praktis untuk banyak tile, tapi bisa untuk testing

### Cara Dashboard Deteksi Offline:

```javascript
// Otomatis deteksi
const mapUrl = navigator.onLine ? 
               'https://{s}.tile.opentopomap.org/{z}/{x}/{y}.png' : 
               'tiles/{z}/{x}/{y}.png';
```

**Dashboard akan:**
- ✅ Gunakan **online tiles** jika ada internet
- ✅ Fallback ke **lokal `/tiles/`** jika offline
- ✅ Update status di info panel: "Offline - Peta Lokal"

---

## 🎛️ LANGKAH 4: Penggunaan Dashboard (New Features)

### Hiker Selector Dropdown:
```html
Pilih Pendaki: [Dropdown ▼]
```

**Cara Kerja:**
1. Ketika ESP32 Bella mengirim data → Otomatis muncul "BELLA" di dropdown
2. Ketika ESP32 Budi mengirim data → Otomatis muncul "BUDI" di dropdown
3. Pilih dropdown → Dashboard update untuk menampilkan data user terpilih
4. Marker & polyline otomatis berganti warna per user

### Status Indikator:
- 🟢 **Aktif** - Terhubung Firebase
- 🟠 **Offline** - Peta lokal aktif, data lama
- 🔴 **Tidak ada data** - Menunggu device

---

## 📊 Struktur Data Firebase (Baru)

### Lama ❌:
```json
{
  "hiker": {
    "lat": -7.5,
    "lon": 110.5
  }
}
```

### Baru ✅:
```json
{
  "hikers": {
    "bella": {
      "lat": -7.5,
      "lon": 110.5,
      "timestamp": 1234567890
    },
    "budi": {
      "lat": -7.6,
      "lon": 110.6,
      "timestamp": 1234567890
    },
    "rina": {
      "lat": -7.4,
      "lon": 110.4,
      "timestamp": 1234567890
    }
  }
}
```

---

## 🎨 Warna Per Hiker

Setiap pendaki punya warna unik:
```javascript
const hikerColors = {
  'bella': '#FC4C02',      // Orange (Strava)
  'budi': '#1E88E5',       // Blue
  'rina': '#43A047',       // Green
  'ricky': '#FB8C00',      // Deep Orange
  'default': '#FC4C02'
};
```

Anda bisa custom di `index.html` line ~380.

---

## ✅ Checklist Implementasi

### Persiapan
- [ ] Backup file original ESP8266 & ESP32
- [ ] Backup file HTML original

### Update Code
- [ ] Update `/hikers/...` path di ESP8266 Gateway
- [ ] Tambah nama pendaki di ESP32 Node
- [ ] Verifikasi format data LoRa: `"bella,-7.5,110.5"`

### Testing
- [ ] Buka Web Dashboard di browser
- [ ] Pastikan dropdown muncul setelah device kirim data
- [ ] Pilih hiker berbeda → dashboard update otomatis
- [ ] Matikan WiFi → cek apakah map fallback ke offline (jika sudah setup `/tiles`)

### Production
- [ ] Deploy `/tiles` folder ke server (jika offline perlu)
- [ ] Update HIKER_NAME untuk setiap ESP32 device
- [ ] Test multi-device tracking

---

## 🐛 Troubleshooting

### Dropdown tidak muncul:
- ❌ Device tidak kirim data (cek LoRa/Serial)
- ❌ Path Firebase salah (harus `/hikers/nama`)
- ✅ Buka DevTools → Console → cek error

### Marker tidak bergerak:
- ❌ activeHikerId null (dropdown belum dipilih)
- ❌ Firebase listener tidak aktif
- ✅ Reload halaman, cek Firebase credentials

### Offline map tidak bekerja:
- ❌ Folder `/tiles` belum dibuat
- ❌ Tile file tidak compatible
- ✅ Gunakan QGIS atau script Python untuk download

---

## 📝 Contoh Data Testing (Manual)

Jika ingin test tanpa device, buka Firebase Console dan buat struktur:

```json
{
  "hikers": {
    "bella": {
      "lat": "-7.50",
      "lon": "110.50"
    },
    "budi": {
      "lat": "-7.51",
      "lon": "110.51"
    }
  }
}
```

Dropdown otomatis muncul dan Anda bisa test UI!

---

## 🎯 Kesimpulan

Dashboard sekarang siap untuk:
1. ✅ **Multiple hikers** tracking real-time
2. ✅ **Offline operation** dengan local tiles
3. ✅ **Individual statistics** per hiker
4. ✅ **Beautiful UI** dengan Strava-style design

Silakan update ESP8266/ESP32 sesuai panduan di atas dan test! 🚀

---

*Last Updated: 2026-01-19*
*For Skripsi: SmartHiker Multi-User IoT Tracking System*
