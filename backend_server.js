const express = require('express');
const cors = require('cors');
const WebSocket = require('ws');
const http = require('http');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// ========== CONFIGURATION ==========
const DASHBOARD_PATH = path.join(__dirname, 'src/Web_Dashboard');
const PORT = process.env.PORT || 3000;

// ========== MIDDLEWARE ==========
app.use(express.static(DASHBOARD_PATH));
app.use(cors());
app.use(express.json());

// ========== IN-MEMORY DATA STORE ==========
let hikersData = {};
let pathHistory = {};

// ========== WEBSOCKET BROADCAST FUNCTION ==========
function broadcastToClients(message) {
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  });
}

// ========== WEBSOCKET CONNECTION ==========
wss.on('connection', (ws) => {
  console.log(`[WebSocket] Client connected. Total clients: ${wss.clients.size}`);
  
  // Send initial data to new client
  ws.send(JSON.stringify({
    type: 'init',
    data: hikersData,
    pathHistory: pathHistory
  }));

  ws.on('close', () => {
    console.log(`[WebSocket] Client disconnected. Total clients: ${wss.clients.size}`);
  });

  ws.on('error', (err) => {
    console.error('[WebSocket] Error:', err.message);
  });
});

// ========== REST ROUTES ==========

// Root: Serve Dashboard
app.get('/', (req, res) => {
  res.sendFile(path.join(DASHBOARD_PATH, 'index.html'));
});

// POST: Receive hiker data dari Gateway
app.post('/api/hiker-data', (req, res) => {
  const { hikerName, latitude, longitude, altitude, temperature, humidity } = req.body;
  
  if (!hikerName || latitude === undefined || longitude === undefined) {
    return res.status(400).json({ error: 'Missing required fields' });
  }

  const hikerKey = hikerName.toLowerCase();
  
  // Initialize if new hiker
  if (!hikersData[hikerKey]) {
    hikersData[hikerKey] = {
      name: hikerName,
      path: [],
      stats: {
        totalDistance: 0,
        maxAltitude: 0,
        minAltitude: 999999,
        avgTemp: 0
      }
    };
    pathHistory[hikerKey] = [];
    console.log(`[New Hiker] ${hikerName} added to tracking`);
  }

  // Update current position & sensor data
  const oldPosition = hikersData[hikerKey].position;
  hikersData[hikerKey].position = {
    lat: parseFloat(latitude),
    lon: parseFloat(longitude),
    timestamp: Date.now()
  };
  hikersData[hikerKey].lastUpdate = Date.now();
  hikersData[hikerKey].altitude = parseFloat(altitude) || 0;
  hikersData[hikerKey].temperature = parseFloat(temperature) || 0;
  hikersData[hikerKey].humidity = parseFloat(humidity) || 0;

  // Calculate distance & update stats
  if (oldPosition) {
    const distance = calculateDistance(oldPosition.lat, oldPosition.lon, latitude, longitude);
    hikersData[hikerKey].stats.totalDistance += distance;
  }
  hikersData[hikerKey].stats.maxAltitude = Math.max(
    hikersData[hikerKey].stats.maxAltitude,
    hikersData[hikerKey].altitude
  );
  hikersData[hikerKey].stats.minAltitude = Math.min(
    hikersData[hikerKey].stats.minAltitude,
    hikersData[hikerKey].altitude
  );

  // Add to path history
  hikersData[hikerKey].path.push({
    lat: latitude,
    lon: longitude,
    alt: altitude,
    timestamp: Date.now()
  });
  pathHistory[hikerKey].push({
    lat: latitude,
    lon: longitude,
    alt: altitude,
    temp: temperature,
    humidity: humidity,
    timestamp: Date.now()
  });

  // Keep only last 1000 points in memory
  if (hikersData[hikerKey].path.length > 1000) {
    hikersData[hikerKey].path.shift();
  }
  if (pathHistory[hikerKey].length > 1000) {
    pathHistory[hikerKey].shift();
  }

  console.log(`[Data Received] ${hikerName}: (${latitude}, ${longitude}) - Alt: ${altitude}m - Temp: ${temperature}°C`);

  // Broadcast update to all clients
  broadcastToClients({
    type: 'update',
    hikerName: hikerKey,
    data: hikersData[hikerKey]
  });

  // Response
  res.json({
    success: true,
    message: 'Data received and broadcasted',
    hikerName: hikerKey,
    currentClients: wss.clients.size
  });
});

// GET: All hikers data
app.get('/api/all-hikers', (req, res) => {
  res.json({
    hikers: hikersData,
    totalHikers: Object.keys(hikersData).length,
    timestamp: Date.now()
  });
});

// GET: Specific hiker data
app.get('/api/hiker/:name', (req, res) => {
  const hikerKey = req.params.name.toLowerCase();
  const hiker = hikersData[hikerKey];
  
  if (!hiker) {
    return res.status(404).json({ error: 'Hiker not found' });
  }
  
  res.json({
    hiker: hiker,
    pathHistory: pathHistory[hikerKey]
  });
});

// GET: Hiker path history
app.get('/api/hiker/:name/path', (req, res) => {
  const hikerKey = req.params.name.toLowerCase();
  const path = pathHistory[hikerKey];
  
  if (!path) {
    return res.status(404).json({ error: 'Hiker not found' });
  }
  
  res.json({
    hikerName: hikerKey,
    pathCount: path.length,
    path: path
  });
});

// GET: Health check
app.get('/api/health', (req, res) => {
  res.json({
    status: 'online',
    uptime: process.uptime(),
    hikers: Object.keys(hikersData),
    wsClients: wss.clients.size,
    timestamp: Date.now()
  });
});

// ========== UTILITY FUNCTIONS ==========
function calculateDistance(lat1, lon1, lat2, lon2) {
  const R = 6371; // Earth radius in km
  const dLat = (lat2 - lat1) * Math.PI / 180;
  const dLon = (lon2 - lon1) * Math.PI / 180;
  const a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
            Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
            Math.sin(dLon / 2) * Math.sin(dLon / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

// ========== ERROR HANDLING ==========
app.use((err, req, res, next) => {
  console.error('[Error]', err.message);
  res.status(500).json({
    error: 'Internal server error',
    message: err.message
  });
});

// ========== START SERVER ==========
server.listen(PORT, '0.0.0.0', () => {
  console.log('\n╔════════════════════════════════════════╗');
  console.log('║  SmartHiker Web Server v1.0           ║');
  console.log('╚════════════════════════════════════════╝\n');
  console.log(`🌐 Dashboard:     http://localhost:${PORT}`);
  console.log(`📡 WebSocket:     ws://localhost:${PORT}`);
  console.log(`📊 API:           http://localhost:${PORT}/api\n`);
  console.log('Endpoints:');
  console.log('  POST   /api/hiker-data          - Receive GPS data');
  console.log('  GET    /api/all-hikers          - Get all hikers');
  console.log('  GET    /api/hiker/:name         - Get specific hiker');
  console.log('  GET    /api/hiker/:name/path    - Get path history');
  console.log('  GET    /api/health              - Health check\n');
  console.log(`Waiting for connections on port ${PORT}...`);
});

// ========== GRACEFUL SHUTDOWN ==========
process.on('SIGINT', () => {
  console.log('\n[Shutdown] Closing server...');
  wss.clients.forEach(client => client.close());
  server.close(() => {
    console.log('[Shutdown] Server closed');
    process.exit(0);
  });
});
