# 🌐 WEB DASHBOARD - START HERE!

## ✨ Apa yang telah selesai?

Semua sudah siap untuk menjalankan web dashboard dengan real-time tracking!

### ✅ Completed
- Backend server code (`backend_server.js`)
- NPM configuration (`package.json`)
- ESP8266 gateway integration code
- Web dashboard (existing `index.html`)
- Complete documentation
- C++ IntelliSense fix

---

## 🚀 Quick Start (5 Minutes)

### 1. Install Node.js
- Download: [nodejs.org](https://nodejs.org) (LTS version)
- Install dengan default settings

### 2. Open Terminal & Navigate
```bash
cd C:\IOT\coba1
```

### 3. Install Dependencies
```bash
npm install
```
(Tunggu ~1-2 menit)

### 4. Start Backend Server
```bash
npm start
```

Output should show:
```
╔════════════════════════════════════════╗
║  SmartHiker Web Server v1.0           ║
╚════════════════════════════════════════╝

🌐 Dashboard:     http://localhost:3000
📡 WebSocket:     ws://localhost:3000
```

### 5. Open Dashboard
Buka browser: **http://localhost:3000**

**Done! Dashboard siap.** ✅

---

## 🧪 Test Tanpa Hardware

Kirim data dummy dari PowerShell:

```powershell
# Copy & paste ini di PowerShell:
$body = @{
    hikerName = "bella"
    latitude = -7.50
    longitude = 110.50
    altitude = 500
    temperature = 25
    humidity = 60
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://localhost:3000/api/hiker-data" `
  -Method POST `
  -Headers @{"Content-Type"="application/json"} `
  -Body $body
```

Lihat dashboard → marker harus muncul! 🎉

---

## 📱 Integrasi dengan Hardware

### Step 1: Cari IP Laptop Anda
```powershell
ipconfig
```
Catat **IPv4 Address** (contoh: `192.168.1.100`)

### Step 2: Update ESP8266 Gateway Code

Edit file: `src/Proyek_Gateway_ESP8266/src/main_gateway_with_backend.cpp`

Ubah 3 baris ini:
```cpp
#define BACKEND_HOST "192.168.1.100"   // ← IP LAPTOP ANDA
#define WIFI_SSID "YOUR_SSID"          // ← WiFi ANDA
#define WIFI_PASSWORD "YOUR_PASSWORD"  // ← Password WiFi
```

### Step 3: Upload ke ESP8266
```bash
cd C:\IOT\coba1\src\Proyek_Gateway_ESP8266
platformio run --target upload
```

### Step 4: Power On Devices
1. ESP32 nodes
2. ESP8266 gateway
3. Check Serial Monitor → harus kirim data

### Step 5: Monitor Dashboard
Lihat real-time updates di `http://localhost:3000`

---

## 📊 Dashboard Features

✅ **Real-time Tracking** - Live marker untuk setiap hiker  
✅ **Interactive Map** - Zoom, pan, fullscreen  
✅ **Elevation Chart** - Visualize altitude profile  
✅ **Statistics** - Distance, speed, temperature, humidity  
✅ **Multi-Hiker** - Track multiple people simultaneously  
✅ **Mobile Responsive** - Works on phone/tablet  
✅ **Offline Mode** - Works without internet  

---

## 🎯 Architecture

```
ESP32 Nodes (LoRa)
        ↓
ESP8266 Gateway (WiFi)
        ↓
Backend Server (Node.js)
        ↓
Web Dashboard (Browser)
```

---

## 📁 Important Files

| File | Purpose |
|------|---------|
| `backend_server.js` | Main backend server - RUN THIS |
| `package.json` | Node.js dependencies |
| `src/Web_Dashboard/index.html` | Frontend dashboard |
| `src/Proyek_Gateway_ESP8266/src/main_gateway_with_backend.cpp` | ESP8266 code |

---

## 🔗 Access from Other Devices

Jika backend running di `192.168.1.100`:

```
Smartphone: http://192.168.1.100:3000
Tablet:     http://192.168.1.100:3000
PC Lain:    http://192.168.1.100:3000
```

(Pastikan semua device di WiFi yang sama!)

---

## 🛠️ Troubleshooting

**"npm: command not found"**
- Install Node.js dari nodejs.org

**"Port 3000 already in use"**
- Gunakan port lain: `PORT=8000 npm start`

**"Cannot connect to server"**
- Pastikan backend masih running
- Refresh browser (Ctrl+R)

**"No real-time updates"**
- Check WebSocket di DevTools (F12)
- Check browser console untuk errors

**"ESP8266 not sending data"**
- Check WiFi connection
- Verify IP address di code
- Check Serial Monitor output

---

## 📚 Documentation Files

1. **This file** (README_WEB.md) ← You are here
2. `WEB_IMPLEMENTATION_SUMMARY.md` - Summary
3. `QUICK_START_WEB.md` - More options
4. `WEB_DOCUMENTATION.md` - Full API reference
5. `WEB_FLOW_DIAGRAM.md` - System architecture
6. `WEB_FILES_SUMMARY.md` - All files created

---

## ✅ Checklist

- [ ] Install Node.js
- [ ] Run `npm install`
- [ ] Run `npm start`
- [ ] Open `http://localhost:3000`
- [ ] Test dengan curl/PowerShell
- [ ] Get your IP address
- [ ] Update ESP8266 code
- [ ] Upload to ESP8266
- [ ] Test dengan hardware
- [ ] Access dari device lain

---

## 🎉 What's Next?

**Immediately**
1. Follow 5-step Quick Start above
2. Test dashboard
3. Test with dummy data

**This Week**
1. Integrate ESP8266 gateway
2. Test hardware flow
3. Monitor real data

**This Month**
1. Optimize performance
2. Deploy to cloud (optional)
3. Add database (optional)

---

## 💡 Tips

- Backend runs on **port 3000** by default
- Dashboard auto-refreshes on data arrival
- All browsers connected see **same data** in real-time
- Works on mobile, tablet, desktop
- Can access from any device on same WiFi

---

## 🚀 Ready to Go!

```bash
npm install    # Install dependencies
npm start      # Start backend
# Open: http://localhost:3000
```

**That's it! Good luck! 🎉**

---

**Questions?** Check documentation files or troubleshooting section above.

**Status**: ✅ Complete & Ready  
**Version**: 1.0  
**Last Updated**: January 19, 2026
